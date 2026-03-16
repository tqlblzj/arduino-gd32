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

#include "vfs_api.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "WString.h"

using namespace fs;

FileImplPtr VFSImpl::open(const char *fpath, const char *mode, const bool create) {
  auto fileImpl = std::make_shared<VFSFileImpl>(this, fpath, mode);
  if (!*fileImpl) {
    printf("Error opening file %s\n", fpath);
    return nullptr;
  }
  return fileImpl;
}

bool VFSImpl::exists(const char *fpath) {
  return lfs_state(fpath, NULL) == LFS_ERR_OK;
}

bool VFSImpl::rename(const char *pathFrom, const char *pathTo) {
  lfs_t* lfs = lfs_get_fs();
  if (lfs == NULL || pathFrom == NULL || pathTo == NULL) {
    return false;
  }

  int ret = lfs_rename_file(pathFrom, pathTo);

  return ret == LFS_ERR_OK;
}

bool VFSImpl::remove(const char *fpath) {
  lfs_t* lfs = lfs_get_fs();
  if (lfs == NULL || fpath == NULL) {
    return false;
  }
  int err = lfs_file_delete(fpath);
  return err == LFS_ERR_OK;
}

bool VFSImpl::mkdir(const char *fpath) {
  lfs_t* lfs = lfs_get_fs();
  if (lfs == NULL || fpath == NULL) {
    return false;
  }
  return lfs_mk_dir(fpath) == LFS_ERR_OK;
}

bool VFSImpl::rmdir(const char *fpath) {
  lfs_t* lfs = lfs_get_fs();
  if (lfs == NULL || fpath == NULL) {
    return false;
  }
  return lfs_rm_dir(fpath) == LFS_ERR_OK;
}

int VFSImpl::state(const char* path, void* info){
  return lfs_state(path, (lfs_info *)info);
}

VFSFileImpl::VFSFileImpl(VFSImpl *fs, const char *fpath, const char *mode)
    : _fs(fs), _file(NULL), _opened(false) {

  strncpy(_file_path, fpath, sizeof(_file_path) - 1);
  _file_path[sizeof(_file_path) - 1] = '\0';
  _path = strdup(fpath);

  // Check if parent directory exists, create if not
  String parentPath = String(fpath);
  int lastSlash = parentPath.lastIndexOf('/');
  if (lastSlash > 0) {
    parentPath = parentPath.substring(0, lastSlash);
  }
  if (parentPath.length() > 0 && !fs->exists(parentPath.c_str())) {
    printf("Creating parent directory: %s\n", parentPath.c_str());
    fs->mkdir(parentPath.c_str());
  }

  _file = (lfs_file_t *)sys_malloc(sizeof(lfs_file_t));
  if (_file == NULL) {
    printf("Failed to allocate memory for file handle\n");
    return;
  }

  sys_memset(_file, 0, sizeof(lfs_file_t));

  int flags = 0;
  if (strcmp(mode, FILE_WRITE) == 0) {
    flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC;
  } else if (strcmp(mode, FILE_READ) == 0) {
    flags = LFS_O_RDONLY;
  } else if (strcmp(mode, FILE_APPEND) == 0) {
    flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND;
  } else {
    flags = LFS_O_RDONLY;
  }

  int err = lfs_open(_file, fpath, flags);
  if (err) {
    printf("Failed to open file %s: %d\n", fpath, err);
    sys_mfree(_file);
    _file = NULL;
    _opened = false;
  } else {
    printf("Opened file %s\n", _path);
    _opened = true;
  }
}

VFSFileImpl::~VFSFileImpl() {
  close();
}

void VFSFileImpl::close() {
  if (_opened && _file != NULL) {
    lfs_close(_file);
    _opened = false;
    sys_mfree(_file);
    _file = NULL;
  }
}

VFSFileImpl::operator bool() {
  return _opened;
}


size_t VFSFileImpl::write(const uint8_t *buf, size_t size) {
  if (!_opened || _file == NULL) {
    return 0;
  }
  uint32_t write_len = 0;
  lfs_write(_file, (uint8_t *)buf, size, &write_len);
  return write_len;
}

size_t VFSFileImpl::read(uint8_t *buf, size_t size) {
  if (!_opened || _file == NULL) {
    return 0;
  }
  uint32_t read_len = 0;
  lfs_read(_file, buf, size, &read_len);
  return read_len;
}

void VFSFileImpl::flush() {
  if (_opened && _file != NULL) {
    lfs_sync(_file);
  }
}

bool VFSFileImpl::seek(uint32_t pos, SeekMode mode) {
  if (!_opened || _file == NULL) {
    return false;
  }

  int whence = LFS_SEEK_SET;
  if (mode == SeekSet) {
    whence = LFS_SEEK_SET;
  } else if (mode == SeekCur) {
    whence = LFS_SEEK_CUR;
  } else if (mode == SeekEnd) {
    whence = LFS_SEEK_END;
  }
  return lfs_lseek(_file, pos, whence) >= 0;
}

size_t VFSFileImpl::position() const {
  if (!_opened || _file == NULL) {
    return 0;
  }
  return lfs_tell(_file);
}

size_t VFSFileImpl::size() const {
  if (!_opened || _file == NULL) {
    return 0;
  }
  return lfs_size(_file);
}

const char *VFSFileImpl::path() const {
  return (const char *)_path;
}

const char *pathToFileName(const char *path) {
  size_t i = 0;
  size_t pos = 0;
  char *p = (char *)path;
  while (*p) {
    i++;
    if (*p == '/' || *p == '\\') {
      pos = i;
    }
    p++;
  }
  return path + pos;
}

const char *VFSFileImpl::name() const {
  return pathToFileName(path());
}

bool VFSFileImpl::isDirectory(void){
  lfs_info info;
  int err = lfs_state(path(), &info);
  if (err != LFS_ERR_OK) {
    return false;
  }
  return info.type == LFS_TYPE_DIR;
}

