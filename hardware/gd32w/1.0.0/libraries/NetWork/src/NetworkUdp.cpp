/*
  Udp.cpp - UDP class for Raspberry Pi
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

#include "NetworkUdp.h"
#include <new>  //std::nothrow
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <errno.h>
#include "wrapper_os.h"

#undef write
#undef read

#define UDP_RX_BUFFER_SIZE 1460

NetworkUDP::NetworkUDP() : udp_server(-1), server_port(0), remote_port(0),
    tx_buffer(0), tx_buffer_len(0),
    rx_buf_data(0), rx_buf_len(0), rx_buf_read_pos(0) {}

NetworkUDP::~NetworkUDP() {
  stop();
}

void NetworkUDP::stop() {
  if (tx_buffer) {
    sys_mfree(tx_buffer);
    tx_buffer = NULL;
  }
  tx_buffer_len = 0;

  // Free fixed-size rx buffer
  if (rx_buf_data) {
    sys_mfree(rx_buf_data);
    rx_buf_data = NULL;
    rx_buf_len = 0;
    rx_buf_read_pos = 0;
  }

  // Free cbuf if exists
  if (rx_buffer) {
    cbuf *b = rx_buffer;
    rx_buffer = NULL;
    sys_mfree(b);
  }

  if (udp_server == -1) {
    return;
  }
  ip_addr_t addr;
  multicast_ip.to_ip_addr_t(&addr);
  if (!ip_addr_isany(&addr)) {
#if CONFIG_IPV6_SUPPORT
    if (multicast_ip.type() == IPv6) {
      struct ipv6_mreq mreq;
      inet6_addr_from_ip6addr(&mreq.ipv6mr_multiaddr, ip_2_ip6(&addr));

      // iterate on each interface
      for (netif *intf = netif_list; intf != nullptr; intf = intf->next) {
        mreq.ipv6mr_interface = intf->num + 1;
        if (intf->name[0] != 'l' || intf->name[1] != 'o') {  // skip 'lo' local interface
          setsockopt(udp_server, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq, sizeof(mreq));
        }
      }
    } else
#endif
    {
      struct ip_mreq mreq;
      mreq.imr_multiaddr.s_addr = (in_addr_t)multicast_ip;
      mreq.imr_interface.s_addr = (in_addr_t)0;
      setsockopt(udp_server, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    }
    // now common code for v4/v6
    multicast_ip = IPAddress();
  }
  close(udp_server);
  udp_server = -1;
}

int NetworkUDP::beginPacket(IPAddress ip, uint16_t port) {
  remote_ip = ip;
  remote_port = port;
  return beginPacket();
}

int NetworkUDP::beginPacket(const char *host, uint16_t port) {
  struct hostent *server;
  server = gethostbyname(host);
  if (server == NULL) {
    printf("[NetworkUDP] could not get host from dns: %d\n", errno);
    return 0;
  }
  return beginPacket(IPAddress((const uint8_t *)(server->h_addr_list[0])), port);
}

int NetworkUDP::beginPacket() {
  if (!remote_port) {
    return 0;
  }

  // allocate tx_buffer if is necessary
  if (!tx_buffer) {
    tx_buffer = (char *)sys_malloc(1460);
    if (!tx_buffer) {
      printf("[NetworkUDP] could not create tx buffer: %d\n", errno);
      return 0;
    }
  }
  tx_buffer_len = 0;

  // check whereas socket is already open
  if (udp_server != -1) {
    return 1;
  }

  if ((udp_server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    printf("[NetworkUDP] could not create socket: %d\n", errno);
    return 0;
  }

  fcntl(udp_server, F_SETFL, O_NONBLOCK);

  return 1;
}

int NetworkUDP::beginMulticastPacket() {
  if (!server_port || multicast_ip == IPAddress()) {
    return 0;
  }
  remote_ip = multicast_ip;
  remote_port = server_port;
  return beginPacket();
}


size_t NetworkUDP::write(uint8_t data) {
  if (tx_buffer_len == 1460) {
    endPacket();
    tx_buffer_len = 0;
  }
  tx_buffer[tx_buffer_len++] = data;
  return 1;
}

size_t NetworkUDP::write(const uint8_t *buffer, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) {
    write(buffer[i]);
  }
  return i;
}

int NetworkUDP::endPacket() {
  ip_addr_t addr;
  remote_ip.to_ip_addr_t(&addr);

  if (remote_ip.type() == IPv4) {
    struct sockaddr_in recipient;
    recipient.sin_addr.s_addr = (uint32_t)remote_ip;
    recipient.sin_family = AF_INET;
    recipient.sin_port = htons(remote_port);
    int sent = sendto(udp_server, tx_buffer, tx_buffer_len, 0, (struct sockaddr *)&recipient, sizeof(recipient));
    if (sent < 0) {
      printf("[NetworkUDP] could not send data: %d\n", errno);
      return 0;
    }
#if CONFIG_IPV6_SUPPORT
  } else {
    struct sockaddr_in6 recipient;
    recipient.sin6_flowinfo = 0;
    recipient.sin6_addr = *(in6_addr *)(ip_addr_t *)(&addr);
    recipient.sin6_family = AF_INET6;
    recipient.sin6_port = htons(remote_port);
    recipient.sin6_scope_id = remote_ip.zone();
    int sent = sendto(udp_server, tx_buffer, tx_buffer_len, 0, (struct sockaddr *)&recipient, sizeof(recipient));
    if (sent < 0) {
      printf("[NetworkUDP] could not send data: %d\n", errno);
      return 0;
    }
#endif
  }
  return 1;
}

void NetworkUDP::flush() {
  clear();
}

void NetworkUDP::clear() {
  // Clear fixed buffer
  rx_buf_len = 0;
  rx_buf_read_pos = 0;

  // Clear cbuf if exists
  if (rx_buffer) {
    cbuf *b = rx_buffer;
    rx_buffer = 0;
    sys_mfree(b);
  }
}

int NetworkUDP::available() {
  // Using fixed buffer
  if (rx_buf_data) {
    return rx_buf_len - rx_buf_read_pos;
  }
  // Using cbuf (legacy support)
  if (rx_buffer) {
    return rx_buffer->available();
  }
  return 0;
}

int NetworkUDP::read() {
  // Using fixed buffer
  if (rx_buf_data) {
    if (rx_buf_read_pos >= rx_buf_len) {
      return -1;
    }
    return rx_buf_data[rx_buf_read_pos++];
  }
  // Using cbuf (legacy support)
  if (rx_buffer) {
    int out = rx_buffer->read();
    if (!rx_buffer->available()) {
      cbuf *b = rx_buffer;
      rx_buffer = 0;
      sys_mfree(b);
    }
    return out;
  }
  return -1;
}

int NetworkUDP::read(unsigned char *buffer, size_t len) {
  return read((char *)buffer, len);
}

int NetworkUDP::read(char *buffer, size_t len) {
  // Using fixed buffer
  if (rx_buf_data) {
    size_t avail = rx_buf_len - rx_buf_read_pos;
    size_t toRead = (len < avail) ? len : avail;

    for (size_t i = 0; i < toRead; i++) {
      buffer[i] = rx_buf_data[rx_buf_read_pos++];
    }
    return toRead;
  }
  // Using cbuf (legacy support)
  if (rx_buffer) {
    int out = rx_buffer->read(buffer, len);
    if (!rx_buffer->available()) {
      cbuf *b = rx_buffer;
      rx_buffer = 0;
      sys_mfree(b);
    }
    return out;
  }
  return 0;
}

int NetworkUDP::peek() {
  // Using fixed buffer
  if (rx_buf_data) {
    if (rx_buf_read_pos >= rx_buf_len) {
      return -1;
    }
    return rx_buf_data[rx_buf_read_pos];
  }
  // Using cbuf (legacy support)
  if (rx_buffer) {
    return rx_buffer->peek();
  }
  return -1;
}

IPAddress NetworkUDP::remoteIP() {
  return remote_ip;
}

uint16_t NetworkUDP::remotePort() {
  return remote_port;
}

/**
 * @brief Read all available data from UDP packet as a String
 *
 * UDP is packet-based, not stream-based. This method reads all available
 * data from the current UDP packet without waiting for timeout.
 *
 * @return String containing all data from the current UDP packet
 */
String NetworkUDP::readString() {
  String ret;

  // Using fixed buffer
  if (rx_buf_data) {
    size_t avail = rx_buf_len - rx_buf_read_pos;
    if (avail > 0) {
      ret.reserve(avail);
      for (size_t i = 0; i < avail; i++) {
        int c = rx_buf_data[rx_buf_read_pos++];
        if (c >= 0) {
          ret += (char)c;
        }
      }
    }
    // Clear buffer after reading
    rx_buf_len = 0;
    rx_buf_read_pos = 0;
  } else if (rx_buffer) {
    // Using cbuf (legacy support)
    size_t avail = rx_buffer->available();
    if (avail > 0) {
      ret.reserve(avail);
      for (size_t i = 0; i < avail; i++) {
        int c = rx_buffer->read();
        if (c >= 0) {
          ret += (char)c;
        }
      }
    }
    // Free buffer after reading
    cbuf *b = rx_buffer;
    rx_buffer = 0;
    sys_mfree(b);
  }

  return ret;
}

uint8_t NetworkUDP::begin(uint16_t p) {
  return begin(IPAddress(), p);
}

/*
 * Function: parsePacket
 * Description: This function parses incoming UDP packets and stores the data in a buffer.
 *              It handles buffer management, checks for available data, and extracts
 *              sender information (IP and port). The function returns the length of
 *              the received packet or 0 if no data is available or an error occurs.
 *
 * Parameters: None
 *
 * Returns:
 *   int - Length of the received packet if successful, 0 otherwise.
 */

int NetworkUDP::parsePacket() {
  // If rx_buffer exists but is empty, free it first
  if (rx_buffer && rx_buffer->available() == 0) {
    cbuf *b = rx_buffer;
    rx_buffer = 0;
    sys_mfree(b);
  }
  // If rx_buffer still exists (has data), return 0 to wait for user to read
  if (rx_buffer) {
    printf("[NetworkUDP] cbuf buffer has unread data\n");
    return 0;
  }

  // Check if fixed buffer has unread data
  if (rx_buf_data && (rx_buf_len > rx_buf_read_pos)) {
    printf("[NetworkUDP] Fixed buffer has unread data\n");
    return 0;  // Previous packet not fully read yet
  }

  // Ensure UDP socket is initialized
  if (udp_server < 0) {
    printf("[NetworkUDP] UDP socket not initialized\n");
    return 0;
  }

  // Prepare storage for sender address (IPv4/IPv6)
  struct sockaddr_storage si_other_storage;  // enough storage for v4 and v6
  socklen_t slen = sizeof(sockaddr_storage);
  int len = 0;

  // Check if there is data available in the socket buffer
  if (ioctl(udp_server, FIONREAD, &len) == -1) {
    printf("[NetworkUDP] could not check for data in buffer length: %d\n", errno);
    return 0;
  }
  if (!len) {
    return 0;
  }

  // Allocate fixed buffer if not already allocated
  if (!rx_buf_data) {
    rx_buf_data = (char *)sys_malloc(UDP_RX_BUFFER_SIZE);
    if (!rx_buf_data) {
      printf("[NetworkUDP] Failed to allocate fixed rx buffer\n");
      return 0;
    }
  }

  // Receive data from the UDP socket
  if ((len = recvfrom(udp_server, rx_buf_data, UDP_RX_BUFFER_SIZE, MSG_DONTWAIT,
                    (struct sockaddr *)&si_other_storage, (socklen_t *)&slen)) == -1) {
    if (errno == EWOULDBLOCK) {
      return 0;
    }
    printf("[NetworkUDP] could not receive data: %d\n", errno);
    return 0;
  }

  // Update buffer state with received data
  rx_buf_len = len;
  rx_buf_read_pos = 0;

  // Extract sender IP and port based on address family (IPv4 or IPv6)
  if (si_other_storage.ss_family == AF_INET) {
    struct sockaddr_in &si_other = (sockaddr_in &)si_other_storage;
    remote_ip = IPAddress(si_other.sin_addr.s_addr);
    remote_port = ntohs(si_other.sin_port);
  }
#if CONFIG_IPV6_SUPPORT
  else if (si_other_storage.ss_family == AF_INET6) {
    struct sockaddr_in6 &si_other = (sockaddr_in6 &)si_other_storage;
    remote_ip = IPAddress(IPv6, (uint8_t *)&si_other.sin6_addr, si_other.sin6_scope_id);  // force IPv6
    ip_addr_t addr;
    remote_ip.to_ip_addr_t(&addr);
    /* Dual-stack: Unmap IPv4 mapped IPv6 addresses */
    if (remote_ip.type() == IPv6 && ip6_addr_isipv4mappedipv6(ip_2_ip6(&addr))) {
      unmap_ipv4_mapped_ipv6(ip_2_ip4(&addr), ip_2_ip6(&addr));
      IP_SET_TYPE_VAL(addr, IPADDR_TYPE_V4);
      remote_ip.from_ip_addr_t(&addr);
    }
    remote_port = ntohs(si_other.sin6_port);
  } else {
    remote_ip = ip_addr_any.u_addr.ip4.addr;
    remote_port = 0;
  }
#else
  else {
    remote_ip = ip_addr_any.addr;
    remote_port = 0;
  }
#endif  // LWIP_IPV6=1

  return len;
}

uint8_t NetworkUDP::beginMulticast(IPAddress address, uint16_t p) {
  if (begin(IPAddress(), p)) {
    ip_addr_t addr;
    address.to_ip_addr_t(&addr);
    if (ip_addr_ismulticast(&addr)) {
#if CONFIG_IPV6_SUPPORT
      if (address.type() == IPv6) {
        struct ipv6_mreq mreq;
        bool joined = false;
        inet6_addr_from_ip6addr(&mreq.ipv6mr_multiaddr, ip_2_ip6(&addr));

        // iterate on each interface
        for (netif *intf = netif_list; intf != nullptr; intf = intf->next) {
          mreq.ipv6mr_interface = intf->num + 1;
          if (intf->name[0] != 'l' || intf->name[1] != 'o') {  // skip 'lo' local interface
            int ret = setsockopt(udp_server, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
            if (ret >= 0) {
              joined = true;
            }
          }
        }
        if (!joined) {
          printf("[NetworkUDP] could not join igmp: %d\n", errno);
          stop();
          return 0;
        }
      } else
#endif
      {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = (in_addr_t)address;
        mreq.imr_interface.s_addr = INADDR_ANY;
        if (setsockopt(udp_server, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
          printf("[NetworkUDP] could not join igmp: %d\n", errno);
          stop();
          return 0;
        }
      }
      multicast_ip = address;
      return 1;
    }
  }
  return 0;
}


uint8_t NetworkUDP::begin(IPAddress address, uint16_t port) {
  stop();

  server_port = port;

  tx_buffer = (char *)sys_malloc(1460);
  if (!tx_buffer) {
    printf("[NetworkUDP] could not create tx buffer: %d\n", errno);
    return 0;
  }
  tx_buffer_len = 0;

  // Allocate fixed-size rx buffer
  rx_buf_data = (char *)sys_malloc(UDP_RX_BUFFER_SIZE);
  if (!rx_buf_data) {
    printf("[NetworkUDP] Failed to allocate fixed rx buffer\n");
    return 0;
  }
  rx_buf_len = 0;
  rx_buf_read_pos = 0;

#if CONFIG_IPV6_SUPPORT
  if ((udp_server = socket((address.type() == IPv6) ? AF_INET6 : AF_INET, SOCK_DGRAM, 0)) == -1) {
#else
  if ((udp_server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
#endif
    printf("[NetworkUDP] could not create socket: %d\n", errno);
    stop();
    return 0;
  }

  int yes = 1;
  if (setsockopt(udp_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    printf("[NetworkUDP] could not set socket option: %d\n", errno);
    stop();
    return 0;
  }

  struct sockaddr_storage serveraddr = {};
  size_t sock_size = 0;
#if CONFIG_IPV6_SUPPORT
  if (address.type() == IPv6) {
    struct sockaddr_in6 *tmpaddr = (struct sockaddr_in6 *)&serveraddr;
    ip_addr_t addr;
    address.to_ip_addr_t(&addr);
    memset((char *)tmpaddr, 0, sizeof(struct sockaddr_in));
    tmpaddr->sin6_family = AF_INET6;
    tmpaddr->sin6_port = htons(server_port);
    tmpaddr->sin6_scope_id = addr.u_addr.ip6.zone;
    inet6_addr_from_ip6addr(&tmpaddr->sin6_addr, ip_2_ip6(&addr));
    tmpaddr->sin6_flowinfo = 0;
    sock_size = sizeof(sockaddr_in6);
  } else
#endif
  {
    struct sockaddr_in *tmpaddr = (struct sockaddr_in *)&serveraddr;
    memset((char *)tmpaddr, 0, sizeof(struct sockaddr_in));
    tmpaddr->sin_family = AF_INET;
    tmpaddr->sin_port = htons(server_port);
    tmpaddr->sin_addr.s_addr = (in_addr_t)address;
    sock_size = sizeof(sockaddr_in);
  }
  if (bind(udp_server, (sockaddr *)&serveraddr, sock_size) == -1) {
    printf("[NetworkUDP] could not bind socket: %d\n", errno);
    stop();
    return 0;
  }
  fcntl(udp_server, F_SETFL, O_NONBLOCK);
  return 1;
}
