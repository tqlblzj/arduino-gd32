/*
Copyright (c) 2025, GigaDevice Semiconductor Inc.

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
*/

#include "Arduino.h"
#include "mbedtls/base64.h"
#include "base64.h"

/**
 * convert input data to base64
 * @param data const uint8_t *
 * @param length size_t
 * @return String
 */
String base64::encode(const uint8_t *data, size_t length) {
  const size_t buf_size = 256;
  unsigned char out[buf_size];
  String result = "";
  size_t pos = 0;

  while (pos < length) {
    // Base64 encodes 3 bytes to 4 chars
    size_t chunk_size = (length - pos);
    size_t max_bytes = ((buf_size - 1) / 4) * 3;  // Ensure output fits
    if (chunk_size > max_bytes) {
      chunk_size = max_bytes;
    }
    // Round down to multiple of 3 for proper base64 encoding
    if (chunk_size > 3) {
      chunk_size = (chunk_size / 3) * 3;
    }

    size_t olen = 0;
    int ret = mbedtls_base64_encode(out, buf_size, &olen, data + pos, chunk_size);
    if (ret != 0) {
      return String("-FAIL-");
    }
    result += (char *)out;
    pos += chunk_size;
  }
  return result;
}

/**
 * convert input data to base64
 * @param text const String&
 * @return String
 */
String base64::encode(const String &text) {
  return base64::encode((uint8_t *)text.c_str(), text.length());
}

/**
 * decode base64 string
 * @param input const char* - base64 encoded string
 * @param input_len size_t - length of input string
 * @param output char* - output buffer for decoded data
 * @param output_size size_t - size of output buffer
 * @param output_len size_t* - actual length of decoded data
 * @return bool - true on success, false on failure
 */
bool base64::decode(const char *input, size_t input_len, char *output, size_t output_size, size_t *output_len) {
  const size_t buf_size = 256;
  unsigned char out[buf_size];
  size_t total_written = 0;
  size_t pos = 0;

  while (pos < input_len) {
    // Base64 decodes 4 chars to 3 bytes
    size_t chunk_size = input_len - pos;
    size_t max_chars = ((buf_size - 1) / 3) * 4;  // Ensure output fits
    if (chunk_size > max_chars) {
      chunk_size = max_chars;
    }
    // Round down to multiple of 4 for proper base64 decoding
    if (chunk_size > 4) {
      chunk_size = (chunk_size / 4) * 4;
    }

    size_t olen = 0;
    int ret = mbedtls_base64_decode(out, buf_size, &olen, (const unsigned char *)(input + pos), chunk_size);
    if (ret != 0) {
      return false;
    }

    if (total_written + olen > output_size) {
      return false;  // Output buffer too small
    }

    sys_memcpy(output + total_written, out, olen);
    total_written += olen;
    pos += chunk_size;
  }

  // Add null terminator if space available
  if (total_written < output_size) {
    output[total_written] = '\0';
  }

  if (output_len) {
    *output_len = total_written;
  }

  return true;
}
