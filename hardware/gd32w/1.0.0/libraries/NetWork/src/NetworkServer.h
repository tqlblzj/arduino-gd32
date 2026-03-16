/*
  Server.cpp - Server class for Raspberry Pi
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

#ifndef NetworkServer_h
#define NetworkServer_h

#include "Arduino.h"
#include "Server.h"
#include "NetworkClient.h"
#include "IPAddress.h"

#undef accept
#undef write
#undef close

class NetworkServer {
private:
  int sockfd;
  int _accepted_sockfd = -1;
  IPAddress _addr;
  uint16_t _port;
  uint8_t _max_clients;
  bool _listening;
  bool _noDelay = false;

public:
  void listenOnLocalhost() {}

  NetworkServer(uint16_t port = 80, uint8_t max_clients = 4)
    : sockfd(-1), _accepted_sockfd(-1), _addr(), _port(port), _max_clients(max_clients), _listening(false), _noDelay(false) {

  }
  NetworkServer(const IPAddress &addr, uint16_t port = 80, uint8_t max_clients = 4)
    : sockfd(-1), _accepted_sockfd(-1), _addr(addr), _port(port), _max_clients(max_clients), _listening(false), _noDelay(false) {

  }
  ~NetworkServer() {
    end();
  }
  NetworkClient available() __attribute__((deprecated("Renamed to accept().")));
  NetworkClient accept();
  void begin(uint16_t port = 0);
  void begin(uint16_t port, int reuse_enable);
  void setNoDelay(bool nodelay);
  bool getNoDelay();
  bool hasClient();

  void end();
  void close();
  void stop();
  operator bool() {
    return _listening;
  }
  int setTimeout(uint32_t seconds);
};

#endif
