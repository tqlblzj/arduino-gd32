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

#include "cbuf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern "C" {
    // Include GD32 memory management functions
    #include "wrapper_os.h"
}

/**
 * @brief Construct a new circular buffer with specified size
 * @param size Size of buffer in bytes
 */
cbuf::cbuf(size_t size) : next(nullptr), has_peek(false), peek_byte(0) {
    _buf = (uint8_t *)sys_malloc(size);
    if (_buf) {
        printf("cbuf allocated %zu bytes\n", size); // Debugging line
        _size = size;
    } else {
        _size = 0;
    }
    _readPos = 0;
    _writePos = 0;
}

/**
 * @brief Destroy the circular buffer and free allocated memory
 */
cbuf::~cbuf() {
    if (_buf) {
        sys_mfree(_buf);
        printf("cbuf freed %zu bytes\n", _size);
        _buf = nullptr;
    }
    _size = 0;
    _readPos = 0;
    _writePos = 0;
}

/**
 * @brief Add additional space to the buffer
 * @param addSize Number of bytes to add
 * @return New size of the buffer
 */
size_t cbuf::resizeAdd(size_t addSize) {
    if (addSize == 0) {
        return _size;
    }
    return resize(_size + addSize);
}

/**
 * @brief Resize the buffer to new size
 * @param newSize New size in bytes
 * @return New size of the buffer
 */
size_t cbuf::resize(size_t newSize) {
    if (newSize == _size) {
        return _size;
    }

    // Calculate available data
    size_t avail = available();

    // If new size is smaller, we can only keep what fits
    if (newSize < avail) {
        // Remove excess data
        remove(avail - newSize);
        avail = newSize;
    }

    // Reallocate buffer
    uint8_t *newBuf = (uint8_t *)sys_realloc(_buf, newSize);
    if (!newBuf) {
        return _size; // Failed to resize
    }

    // If we have data and it's wrapped, move it to the beginning
    if (avail > 0 && _readPos > _writePos) {
        size_t firstPart = _size - _readPos;
        size_t secondPart = _writePos;

        // Move first part to end of new buffer
        if (newSize >= avail) {
            memmove(newBuf + newSize - firstPart, newBuf + _readPos, firstPart);
            // Move second part to beginning
            memmove(newBuf, newBuf, secondPart);
            // Update positions
            _readPos = newSize - firstPart;
            _writePos = secondPart;
        }
    } else if (avail > 0) {
        // Data is contiguous, just ensure it's at the beginning
        if (_readPos != 0) {
            memmove(newBuf, newBuf + _readPos, avail);
            _readPos = 0;
            _writePos = avail;
        }
    } else {
        // No data, reset positions
        _readPos = 0;
        _writePos = 0;
    }

    _buf = newBuf;
    _size = newSize;

    return _size;
}

/**
 * @brief Get number of bytes available for reading
 * @return Number of bytes available
 */
size_t cbuf::available() const {
    if (_writePos >= _readPos) {
        return _writePos - _readPos;
    } else {
        return _size - _readPos + _writePos;
    }
}

/**
 * @brief Get total size of the buffer
 * @return Total buffer size in bytes
 */
size_t cbuf::size() {
    return _size;
}

/**
 * @brief Get available space for writing
 * @return Number of bytes available for writing
 */
size_t cbuf::room() const {
    return _size - available() - 1; // -1 to distinguish full from empty
}

/**
 * @brief Check if buffer is empty
 * @return true if buffer is empty, false otherwise
 */
bool cbuf::empty() const {
    return _readPos == _writePos;
}

/**
 * @brief Check if buffer is full
 * @return true if buffer is full, false otherwise
 */
bool cbuf::full() const {
    return room() == 0;
}

/**
 * @brief Peek at next byte without removing it
 * @return Next byte, or -1 if buffer is empty
 */
int cbuf::peek() {
    if (empty()) {
        return -1;
    }
    return _buf[_readPos];
}

/**
 * @brief Read one byte from buffer
 * @return Byte read, or -1 if buffer is empty
 */
int cbuf::read() {
    if (empty()) {
        return -1;
    }
    uint8_t byte = _buf[_readPos];
    _readPos = (_readPos + 1) % _size;
    return byte;
}

/**
 * @brief Read multiple bytes from buffer
 * @param dst Destination buffer
 * @param size Number of bytes to read
 * @return Number of bytes actually read
 */
size_t cbuf::read(char *dst, size_t size) {
    size_t avail = available();
    size_t toRead = (size < avail) ? size : avail;

    if (toRead == 0) {
        return 0;
    }

    // Handle wrapped buffer
    if (_readPos + toRead <= _size) {
        // Data is contiguous
        sys_memcpy(dst, _buf + _readPos, toRead);
        _readPos += toRead;
    } else {
        // Data wraps around
        size_t firstPart = _size - _readPos;
        size_t secondPart = toRead - firstPart;
        sys_memcpy(dst, _buf + _readPos, firstPart);
        sys_memcpy(dst + firstPart, _buf, secondPart);
        _readPos = secondPart;
    }

    return toRead;
}

/**
 * @brief Write one byte to buffer
 * @param c Byte to write
 * @return Number of bytes written (1 or 0)
 */
size_t cbuf::write(char c) {
    if (full()) {
        return 0;
    }
    _buf[_writePos] = c;
    _writePos = (_writePos + 1) % _size;
    return 1;
}

/**
 * @brief Write multiple bytes to buffer
 * @param src Source data pointer
 * @param size Number of bytes to write
 * @return Number of bytes actually written
 */
size_t cbuf::write(const char *src, size_t size) {
    size_t avail = room();
    size_t toWrite = (size < avail) ? size : avail;

    if (toWrite == 0) {
        return 0;
    }

    // Handle wrapped buffer
    if (_writePos + toWrite <= _size) {
        // Space is contiguous
        sys_memcpy(_buf + _writePos, src, toWrite);
        _writePos += toWrite;
    } else {
        // Space wraps around
        size_t firstPart = _size - _writePos;
        size_t secondPart = toWrite - firstPart;
        sys_memcpy(_buf + _writePos, src, firstPart);
        sys_memcpy(_buf, src + firstPart, secondPart);
        _writePos = secondPart;
    }

    return toWrite;
}

/**
 * @brief Flush buffer (clear all data)
 */
void cbuf::flush() {
    _readPos = 0;
    _writePos = 0;
}

/**
 * @brief Remove bytes from buffer head
 * @param size Number of bytes to remove
 * @return Number of bytes removed
 */
size_t cbuf::remove(size_t size) {
    size_t avail = available();
    size_t toRemove = (size < avail) ? size : avail;

    _readPos = (_readPos + toRemove) % _size;

    return toRemove;
}
