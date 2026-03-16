/*
FSImpl.h - base file system interface
Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
This file is part of the esp8266 core for Arduino environment.

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

#ifndef FSIMPL_H
#define FSIMPL_H

#include <stddef.h>
#include <stdint.h>

namespace fs {

class FileImpl {
public:
  virtual ~FileImpl() {}
  virtual size_t write(const uint8_t *buf, size_t size) = 0;
  virtual size_t read(uint8_t *buf, size_t size) = 0;
  virtual void flush() = 0;
  virtual bool seek(uint32_t pos, SeekMode mode) = 0;
  virtual size_t position() const = 0;
  virtual size_t size() const = 0;
  virtual void close() = 0;
  virtual const char *path() const = 0;
  virtual const char *name() const = 0;
  virtual bool isDirectory(void) = 0;
};

class FSImpl {
protected:
  bool _mounted;

public:
  FSImpl() : _mounted(false) {}
  virtual ~FSImpl() {}
  virtual FileImplPtr open(const char *path, const char *mode, const bool create) = 0;
  virtual bool exists(const char *path) = 0;
  virtual bool rename(const char *pathFrom, const char *pathTo) = 0;
  virtual bool remove(const char *path) = 0;
  virtual bool mkdir(const char *path) = 0;
  virtual bool rmdir(const char *path) = 0;
  virtual int state(const char* path, void* info) = 0;
  void set_mount(bool mounted){
    _mounted = mounted;
  }

  bool get_mount(){
    return _mounted;
  }
};

}  // namespace fs

#endif  //FSIMPL_H
