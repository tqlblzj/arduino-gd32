/*
Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
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

#ifndef vfs_api_h
#define vfs_api_h

#include "FS.h"
#include "FSImpl.h"
#include "gdlittlefs.h"


using namespace fs;

class VFSFileImpl;

class VFSImpl : public FSImpl {

protected:
  friend class VFSFileImpl;

public:
  FileImplPtr open(const char *path, const char *mode, const bool create) override;
  bool exists(const char *path) override;
  bool rename(const char *pathFrom, const char *pathTo) override;
  bool remove(const char *path) override;
  bool mkdir(const char *path) override;
  bool rmdir(const char *path) override;
  int state(const char* path, void* info) override;
};

class VFSFileImpl : public FileImpl {
protected:
  VFSImpl *_fs;
  lfs_file_t *_file;
  bool _opened;
  char _file_path[256];
  char *_path;

public:
  VFSFileImpl(VFSImpl *fs, const char *path, const char *mode);
  ~VFSFileImpl() override;
  size_t write(const uint8_t *buf, size_t size) override;
  size_t read(uint8_t *buf, size_t size) override;
  void flush() override;
  bool seek(uint32_t pos, SeekMode mode) override;
  size_t position() const override;
  size_t size() const override;
  void close() override;
  const char *path() const override;
  const char *name() const override;
  bool isDirectory(void) override;
  operator bool();
};

#endif

