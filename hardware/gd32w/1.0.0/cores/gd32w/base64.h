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

#ifndef CORE_BASE64_H_
#define CORE_BASE64_H_

#include <stddef.h>

class base64 {
public:
  static String encode(const uint8_t *data, size_t length);
  static String encode(const String &text);
  static bool decode(const char *input, size_t input_len, char *output, size_t output_size, size_t *output_len);

private:
};

#endif /* CORE_BASE64_H_ */
