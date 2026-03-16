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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Circular buffer class for efficient data buffering
 *
 * This class provides a thread-safe circular buffer implementation
 * commonly used for network data buffering in Arduino/GD32VW55x environment.
 */
class cbuf {
public:
    /**
     * @brief Construct a new circular buffer with specified size
     * @param size Size of buffer in bytes
     */
    cbuf(size_t size);

    /**
     * @brief Destroy the circular buffer and free allocated memory
     */
    ~cbuf();

    /**
     * @brief Add additional space to the buffer
     * @param addSize Number of bytes to add
     * @return New size of the buffer
     */
    size_t resizeAdd(size_t addSize);

    /**
     * @brief Resize the buffer to new size
     * @param newSize New size in bytes
     * @return New size of the buffer
     */
    size_t resize(size_t newSize);

    /**
     * @brief Get number of bytes available for reading
     * @return Number of bytes available
     */
    size_t available() const;

    /**
     * @brief Get total size of the buffer
     * @return Total buffer size in bytes
     */
    size_t size();

    /**
     * @brief Get available space for writing
     * @return Number of bytes available for writing
     */
    size_t room() const;

    /**
     * @brief Check if buffer is empty
     * @return true if buffer is empty, false otherwise
     */
    bool empty() const;

    /**
     * @brief Check if buffer is full
     * @return true if buffer is full, false otherwise
     */
    bool full() const;

    /**
     * @brief Peek at next byte without removing it
     * @return Next byte, or -1 if buffer is empty
     */
    int peek();

    /**
     * @brief Read one byte from buffer
     * @return Byte read, or -1 if buffer is empty
     */
    int read();

    /**
     * @brief Read multiple bytes from buffer
     * @param dst Destination buffer
     * @param size Number of bytes to read
     * @return Number of bytes actually read
     */
    size_t read(char *dst, size_t size);

    /**
     * @brief Write one byte to buffer
     * @param c Byte to write
     * @return Number of bytes written (1 or 0)
     */
    size_t write(char c);

    /**
     * @brief Write multiple bytes to buffer
     * @param src Source data pointer
     * @param size Number of bytes to write
     * @return Number of bytes actually written
     */
    size_t write(const char *src, size_t size);

    /**
     * @brief Flush buffer (clear all data)
     */
    void flush();

    /**
     * @brief Remove bytes from buffer head
     * @param size Number of bytes to remove
     * @return Number of bytes removed
     */
    size_t remove(size_t size);

    // Linked list support for chaining buffers
    cbuf *next;
    bool has_peek;
    uint8_t peek_byte;

protected:
    uint8_t *_buf;
    size_t _size;
    size_t _readPos;
    size_t _writePos;
};
