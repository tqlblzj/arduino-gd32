/*
Copyright 2015-2020 Espressif Systems (Shanghai) PTE LTD
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

#include "LittleFS.h"

#include "vfs_api.h"

using namespace fs;

class LittleFSImpl : public VFSImpl {
public:
  LittleFSImpl();
  virtual ~LittleFSImpl() {}
};

LittleFSImpl::LittleFSImpl() {}

LittleFSFS::LittleFSFS() : FS(FSImplPtr(new LittleFSImpl())), _mounted(false) {}

LittleFSFS::~LittleFSFS() {
    end();
}

bool LittleFSFS::begin() {
    if(!lfs_mk_mount()){
        return false;
    }
    _mounted = true;
    _impl->set_mount(true);
    return true;
}

bool LittleFSFS::end() {
    bool res = true;
    if(_mounted){
        res = lfs_mk_unmount();
    }
    _impl->set_mount(false);
    _mounted = false;
    return res;
}

LittleFSFS LittleFS;
