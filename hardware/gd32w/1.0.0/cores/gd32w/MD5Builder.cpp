/*
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.

  This library is sys_mfree software; you can redistribute it and/or
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

#include "HEXBuilder.h"
#include "MD5Builder.h"

void MD5Builder::begin(void) {
  sys_memset(_buf, 0x00, MD5_DIGEST_LEN);
  mbedtls_md5_init(&_ctx);
  mbedtls_md5_starts(&_ctx);
}

void MD5Builder::add(const unsigned char *data, size_t len) {
  mbedtls_md5_update(&_ctx, data, len);
}

bool MD5Builder::addStream(Stream &stream, const size_t maxLen) {
  const int buf_size = 512;
  int maxLengthLeft = maxLen;
  uint8_t buf[buf_size];

  int bytesAvailable = stream.available();
  while ((bytesAvailable > 0) && (maxLengthLeft > 0)) {

    // determine number of bytes to read
    int readBytes = bytesAvailable;
    if (readBytes > maxLengthLeft) {
      readBytes = maxLengthLeft;  // read only until max_len
    }
    if (readBytes > buf_size) {
      readBytes = buf_size;  // not read more the buffer can handle
    }

    // read data and check if we got something
    int numBytesRead = stream.readBytes(buf, readBytes);
    if (numBytesRead < 1) {
      return false;
    }

    // Update MD5 with buffer payload
    mbedtls_md5_update(&_ctx, buf, numBytesRead);

    // update available number of bytes
    maxLengthLeft -= numBytesRead;
    bytesAvailable = stream.available();
  }
  return true;
}

void MD5Builder::calculate(void) {
  mbedtls_md5_finish(&_ctx, _buf);
}

void MD5Builder::getBytes(uint8_t *output) {
  sys_memcpy(output, _buf, MD5_DIGEST_LEN);
}

void MD5Builder::getChars(char *output) {
  bytes2hex(output, MD5_DIGEST_LEN * 2 + 1, _buf, MD5_DIGEST_LEN);
}

String MD5Builder::toString(void) {
  char out[(MD5_DIGEST_LEN * 2) + 1];
  getChars(out);
  return String(out);
}
