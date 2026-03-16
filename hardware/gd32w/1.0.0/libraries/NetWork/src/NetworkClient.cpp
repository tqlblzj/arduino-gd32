/*
  Client.h - Client class for Raspberry Pi
  Copyright (c) 2016 Hristo Gochkov  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 12 Feb 2026 by GigaDevice Semiconductor Inc (adapt to the first version of gd32 arduino sdk).
*/

#include "NetworkClient.h"
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#define WIFI_CLIENT_DEF_CONN_TIMEOUT_MS (3000)
#define WIFI_CLIENT_MAX_WRITE_RETRY     (10)
#define WIFI_CLIENT_SELECT_TIMEOUT_US   (1000000)

// Socket handle wrapper
class NetworkClientSocketHandle {
private:
    int sockfd;

public:
    NetworkClientSocketHandle(int fd) : sockfd(fd) {}

    ~NetworkClientSocketHandle() {
        close();
    }

    void close() {
        if (sockfd >= 0) {
            ::close(sockfd);
            sockfd = -1;
        }
    }

    int fd() {
        return sockfd;
    }
};

// RX buffer for efficient reading
class NetworkClientRxBuffer {
private:
    size_t _size;
    uint8_t *_buffer;
    size_t _pos;
    size_t _fill;
    int _fd;
    bool _failed;

    size_t r_available() {
        if (_fd < 0) {
            return 0;
        }
        int count;
        int res = lwip_ioctl(_fd, FIONREAD, &count);
        if (res < 0) {
            _failed = true;
            return 0;
        }
        return count;
    }

    size_t fillBuffer() {
        if (!_buffer) {
            _buffer = (uint8_t *)sys_malloc(_size);
            if (!_buffer) {
                _failed = true;
                return 0;
            }
        }
        if (_fill && _pos == _fill) {
            _fill = 0;
            _pos = 0;
        }
        if (!_buffer || _size <= _fill || !r_available()) {
            return 0;
        }
        int res = recv(_fd, _buffer + _fill, _size - _fill, MSG_DONTWAIT);
        if (res < 0) {
            if (errno != EWOULDBLOCK && errno != 0) {
                _failed = true;
            }
            return 0;
        } else if (res == 0) {
            // Connection closed
            return 0;
        }
        _fill += res;
        return res;
    }

public:
    NetworkClientRxBuffer(int fd, size_t size = 1436) : _size(size), _buffer(NULL), _pos(0), _fill(0), _fd(fd), _failed(false) {}

    ~NetworkClientRxBuffer() {
        sys_mfree(_buffer);
    }

    bool failed() {
        return _failed;
    }

    int read(uint8_t *dst, size_t len) {
        if (!dst || !len || (_pos == _fill && !fillBuffer())) {
            return _failed ? -1 : 0;
        }
        size_t a = _fill - _pos;
        if (len <= a || ((len - a) <= (_size - _fill) && fillBuffer() >= (len - a))) {
            if (len == 1) {
                *dst = _buffer[_pos];
            } else {
                sys_memcpy(dst, _buffer + _pos, len);
            }
            _pos += len;
            return len;
        }
        size_t left = len;
        size_t toRead = a;
        uint8_t *buf = dst;
        sys_memcpy(buf, _buffer + _pos, toRead);
        _pos += toRead;
        left -= toRead;
        buf += toRead;
        while (left) {
            if (!fillBuffer()) {
                return len - left;
            }
            a = _fill - _pos;
            toRead = (a > left) ? left : a;
            sys_memcpy(buf, _buffer + _pos, toRead);
            _pos += toRead;
            left -= toRead;
            buf += toRead;
        }
        return len;
    }

    int peek() {
        if (_pos == _fill && !fillBuffer()) {
            return -1;
        }
        return _buffer[_pos];
    }

    size_t available() {
        return _fill - _pos + r_available();
    }

    void clear() {
        if (r_available()) {
            _pos = _fill;
            while (fillBuffer()) {
                _pos = _fill;
            }
        }
        _pos = 0;
        _fill = 0;
    }
};

NetworkClient::NetworkClient() : _sockfd(-1), _timeout(WIFI_CLIENT_DEF_CONN_TIMEOUT_MS), _connected(false),
    _socketHandle(nullptr), _rxBuffer(nullptr) {}

NetworkClient::NetworkClient(int sockfd) : _sockfd(sockfd), _timeout(WIFI_CLIENT_DEF_CONN_TIMEOUT_MS), _connected(true),
    _socketHandle(nullptr), _rxBuffer(nullptr) {
    if (sockfd >= 0) {
        _socketHandle.reset(new NetworkClientSocketHandle(sockfd));
        _rxBuffer.reset(new NetworkClientRxBuffer(sockfd));
        if (!_rxBuffer) {
            printf("[NetworkClient] Failed to allocate rxBuffer\n");
        }
    }
}

NetworkClient::NetworkClient(const NetworkClient &other) : _sockfd(other._sockfd), _timeout(other._timeout),
    _connected(other._connected), _socketHandle(other._socketHandle), _rxBuffer(other._rxBuffer) {
}

NetworkClient::NetworkClient(NetworkClient &&other) noexcept : _sockfd(other._sockfd), _timeout(other._timeout),
    _connected(other._connected), _socketHandle(std::move(other._socketHandle)), _rxBuffer(std::move(other._rxBuffer)) {
    other._sockfd = -1;
    other._connected = false;
}

NetworkClient &NetworkClient::operator=(const NetworkClient &other) {
    if (this != &other) {
        _sockfd = other._sockfd;
        _timeout = other._timeout;
        _connected = other._connected;
        _socketHandle = other._socketHandle;
        _rxBuffer = other._rxBuffer;
    }
    return *this;
}

NetworkClient &NetworkClient::operator=(NetworkClient &&other) noexcept {
    if (this != &other) {
        _sockfd = other._sockfd;
        _timeout = other._timeout;
        _connected = other._connected;
        _socketHandle = std::move(other._socketHandle);
        _rxBuffer = std::move(other._rxBuffer);
        other._sockfd = -1;
        other._connected = false;
    }
    return *this;
}

NetworkClient::~NetworkClient() {
    stop();
}

int NetworkClient::connect(IPAddress ip, uint16_t port) {
    struct sockaddr_in serveraddr;
    int sockfd = -1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return 0;
    }

    lwip_fcntl(sockfd, F_SETFL, lwip_fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = (uint32_t)ip;
    serveraddr.sin_port = htons(port);

    int res = lwip_connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (res < 0 && errno != EINPROGRESS) {
        close(sockfd);
        return 0;
    }

    fd_set fdset;
    struct timeval tv;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    tv.tv_sec = _timeout / 1000;
    tv.tv_usec = (_timeout % 1000) * 1000;

    res = select(sockfd + 1, nullptr, &fdset, nullptr, _timeout < 0 ? nullptr : &tv);
    if (res < 0) {
        close(sockfd);
        return 0;
    } else if (res == 0) {
        close(sockfd);
        return 0;
    } else {
        int sockerr;
        socklen_t len = (socklen_t)sizeof(int);
        res = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &sockerr, &len);

        if (res < 0) {
            close(sockfd);
            return 0;
        }

        if (sockerr != 0) {
            close(sockfd);
            return 0;
        }
    }

    struct timeval timeout_tv;
    timeout_tv.tv_sec = _timeout / 1000;
    timeout_tv.tv_usec = (_timeout % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout_tv, sizeof(timeout_tv));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof(timeout_tv));

    lwip_fcntl(sockfd, F_SETFL, lwip_fcntl(sockfd, F_GETFL, 0) & (~O_NONBLOCK));

    _sockfd = sockfd;
    _connected = true;
    _socketHandle.reset(new NetworkClientSocketHandle(_sockfd));
    _rxBuffer.reset(new NetworkClientRxBuffer(_sockfd));

    return 1;
}

int NetworkClient::connect(const char *host, uint16_t port) {
    struct hostent *hp;
    struct in_addr **addr_list;
    IPAddress srv((uint32_t)0);

    hp = gethostbyname(host);
    if (hp == nullptr) {
        return 0;
    }

    addr_list = (struct in_addr **)hp->h_addr_list;
    if (addr_list[0] != nullptr) {
        srv = IPAddress((uint32_t)(addr_list[0]->s_addr));
    } else {
        return 0;
    }

    return connect(srv, port);
}

size_t NetworkClient::write(uint8_t data) {
    return write(&data, 1);
}


size_t NetworkClient::write(Stream &stream) {
    uint8_t *buf = (uint8_t *)malloc(1360);
    if (!buf) {
        return 0;
    }
    size_t toRead = 0, toWrite = 0, written = 0;
    size_t available = stream.available();
    while (available) {
        toRead = (available > 1360) ? 1360 : available;
        toWrite = stream.readBytes(buf, toRead);
        written += write(buf, toWrite);
        available = stream.available();
    }
    sys_mfree(buf);
    return written;
}

size_t NetworkClient::write(const uint8_t *buf, size_t size) {
    int res = 0;
    int retry = WIFI_CLIENT_MAX_WRITE_RETRY;
    int socketFileDescriptor = fd();
    size_t totalBytesSent = 0;
    size_t bytesRemaining = size;

    if (!_connected || (socketFileDescriptor < 0)) {
        return 0;
    }

    while (retry) {
        fd_set set;
        struct timeval tv;
        FD_ZERO(&set);
        FD_SET(socketFileDescriptor, &set);
        tv.tv_sec = 0;
        tv.tv_usec = WIFI_CLIENT_SELECT_TIMEOUT_US;
        retry--;

        if (select(socketFileDescriptor + 1, NULL, &set, NULL, &tv) < 0) {
            return 0;
        }

        if (FD_ISSET(socketFileDescriptor, &set)) {
            res = send(socketFileDescriptor, (void *)buf, bytesRemaining, MSG_DONTWAIT);
            if (res > 0) {
                totalBytesSent += res;
                if (totalBytesSent >= size) {
                    retry = 0;
                } else {
                    buf += res;
                    bytesRemaining -= res;
                    retry = WIFI_CLIENT_MAX_WRITE_RETRY;
                }
            } else if (res < 0) {
                if (errno != EAGAIN) {
                    stop();
                    res = 0;
                    retry = 0;
                }
            } else {
                // Try again
            }
        }
    }
    return totalBytesSent;
}

int NetworkClient::available() {
    if (fd() < 0 || !_rxBuffer) {
        if(!_rxBuffer){
            printf("[NetworkClient] available() called but _rxBuffer is nullptr, fd: %d\n", fd());
        }
        if(fd() < 0){
            printf("[NetworkClient] available() called but fd() < 0, fd: %d\n", fd());
        }

        return 0;
    }
    int res = _rxBuffer->available();
    if (_rxBuffer->failed()) {
        printf("[NetworkClient] available() called but _rxBuffer->failed()!!!!, fd: %d\n", fd());
        stop();
    }
    return res;
}

int NetworkClient::read() {
    uint8_t data = 0;
    int res = read(&data, 1);
    if (res < 0) {
        return res;
    }
    if (res == 0) {
        return -1;
    }
    return data;
}

int NetworkClient::read(uint8_t *buf, size_t size) {
    int res = -1;
    if (_rxBuffer) {
        res = _rxBuffer->read(buf, size);
        if (_rxBuffer->failed()) {
            stop();
        }
    }
    return res;
}

int NetworkClient::peek() {
    int res = -1;
    if (fd() >= 0 && _rxBuffer) {
        res = _rxBuffer->peek();
        if (_rxBuffer->failed()) {
            stop();
        }
    }
    return res;
}

void NetworkClient::flush() {
    clear();
}

void NetworkClient::clear() {
    if (_rxBuffer != nullptr) {
        _rxBuffer->clear();
    }
}

void NetworkClient::stop() {
    if (_socketHandle) {
        _socketHandle->close();
        _socketHandle.reset();
    }
    if (_rxBuffer) {
        _rxBuffer.reset();
    }
    _sockfd = -1;
    _connected = false;
}

uint8_t NetworkClient::connected() {
    if (fd() == -1 && _connected) {
        printf("[NetworkClient] connected(): fd() == -1 && _connected, NOT calling stop(), fd: %d\n", fd());
    }
    if (_connected) {
        uint8_t dummy;
        int res = recv(fd(), &dummy, 1, MSG_DONTWAIT | MSG_PEEK);
        (void)res;
        if (res <= 0) {
            switch (errno) {
                case EWOULDBLOCK:
                case ENOENT:
                    _connected = true;
                    break;
                case ENOTCONN:
                case EPIPE:
                case ECONNRESET:
                case ECONNREFUSED:
                case ECONNABORTED:
                    _connected = false;
                    break;
                default:
                    _connected = true;
                    break;
            }
        } else {
            _connected = true;
        }
    }
    return _connected;
}

bool NetworkClient::operator==(const NetworkClient &rhs) {
    return _sockfd == rhs._sockfd;
}

int NetworkClient::fd() const {
    return _sockfd;
}

void NetworkClient::setConnectionTimeout(uint32_t milliseconds) {
    _timeout = milliseconds;
}

int NetworkClient::setNoDelay(bool nodelay) {
    int flag = nodelay;
    return setsockopt(fd(), IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
}

bool NetworkClient::getNoDelay() {
    int flag = 0;
    socklen_t size = sizeof(int);
    getsockopt(fd(), IPPROTO_TCP, TCP_NODELAY, &flag, &size);
    return flag;
}

IPAddress NetworkClient::remoteIP() const {
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    getpeername(fd(), (struct sockaddr *)&addr, &len);
    return IPAddress((uint32_t)(addr.sin_addr.s_addr));
}

uint16_t NetworkClient::remotePort() const {
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    getpeername(fd(), (struct sockaddr *)&addr, &len);
    return ntohs(addr.sin_port);
}

IPAddress NetworkClient::localIP() const {
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    getsockname(fd(), (struct sockaddr *)&addr, &len);
    return IPAddress((uint32_t)(addr.sin_addr.s_addr));
}

uint16_t NetworkClient::localPort() const {
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    getsockname(fd(), (struct sockaddr *)&addr, &len);
    return ntohs(addr.sin_port);
}


