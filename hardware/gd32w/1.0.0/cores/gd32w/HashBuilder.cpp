/*
Copyright 2024 Espressif Systems (Shanghai) PTE LTD
Copyright (c) 2025, GigaDevice Semiconductor Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "HashBuilder.h"

void HashBuilder::add(const unsigned char *data) {
  add(data, strlen((const char *)data));
}

void HashBuilder::add(String data) {
  const unsigned char *cstr = (const unsigned char *)data.c_str();
  add(cstr, strlen((const char *)cstr));
}

void HashBuilder::addHexString(const char *data) {
  const size_t buf_size = 256;
  uint8_t tmp[buf_size];
  size_t len = strlen(data);
  size_t pos = 0;

  while (pos < len) {
    size_t chunk_len = len - pos;
    size_t max_hex_chars = buf_size * 2;
    if (chunk_len > max_hex_chars) {
      chunk_len = max_hex_chars;
    }
    size_t bytes_to_process = chunk_len / 2;
    hex2bytes(tmp, bytes_to_process, data + pos);
    add(tmp, bytes_to_process);
    pos += chunk_len;
  }
}

void HashBuilder::addHexString(String data) {
  addHexString(data.c_str());
}
