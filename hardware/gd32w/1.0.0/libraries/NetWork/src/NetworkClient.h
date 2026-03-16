/*
  Client.h - Base class that provides Client
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

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

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H


#define LWIP_TIMEVAL_PRIVATE 0

#include "Arduino.h"
#include "Client.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <memory>
#include "wrapper_os.h"

#undef connect
#undef write
#undef read
// #undef close

class NetworkClientSocketHandle;
class NetworkClientRxBuffer;

class NetworkClient : public Client {
private:
    int _sockfd;
    int _timeout;
    bool _connected;
    std::shared_ptr<NetworkClientSocketHandle> _socketHandle;
    std::shared_ptr<NetworkClientRxBuffer> _rxBuffer;

public:
    NetworkClient();
    NetworkClient(int sockfd);
    NetworkClient(const NetworkClient &other);
    NetworkClient(NetworkClient &&other) noexcept;
    NetworkClient &operator=(const NetworkClient &other);
    NetworkClient &operator=(NetworkClient &&other) noexcept;
    ~NetworkClient();

    // Connection methods
    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);
    void stop();

    // Write methods
    size_t write(uint8_t data);
    size_t write(const uint8_t *buf, size_t size);
    size_t write(Stream &stream);

    std::shared_ptr<NetworkClientRxBuffer> getRXBufferPtr(){
        return _rxBuffer;
    }

    // Read methods
    int available();
    int read();
    int read(uint8_t *buf, size_t size);
    int peek();
    void flush();
    void clear();

    // Status methods
    uint8_t connected();

    // Operator overloads
    operator bool() {
        return connected();
    }
    bool operator==(const bool value) {
        return bool() == value;
    }
    bool operator!=(const bool value) {
        return bool() != value;
    }
    bool operator==(const NetworkClient &rhs);
    bool operator!=(const NetworkClient &rhs) {
        return !this->operator==(rhs);
    }

    // Socket options
    int fd() const;
    void setConnectionTimeout(uint32_t milliseconds);
    int setNoDelay(bool nodelay);
    bool getNoDelay();

    // Address info
    IPAddress remoteIP() const;
    uint16_t remotePort() const;
    IPAddress localIP() const;
    uint16_t localPort() const;
};

#endif // NETWORK_CLIENT_H
