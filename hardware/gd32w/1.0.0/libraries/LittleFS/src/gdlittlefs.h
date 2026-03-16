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

#ifndef GD_LITTLEFS_H
#define GD_LITTLEFS_H

#include "app_cfg.h"
#include "wrapper_os.h"
#include "lfs.h"
#include "raw_flash_api.h"
#include "lfs_cfg.h"
#include "config_gdm32.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SIP_FLASH_SECTOR_SIZE       (0x1000)
#define SIP_FALSH_START_ADDR        RE_IMG_1_OFFSET
#define SIP_FULL_MEM_SIZE           (0xC0000)


#define MAX_FILE_STATE 4
#define MAX_FILE_PATH 128
#define MAX_FILE_COUNT 64


bool lfs_mk_mount(void);

bool lfs_mk_unmount(void);

int lfs_mk_dir(const char* path);

int lfs_rm_file(const char* path);
int lfs_rm_dir(const char* path);

int lfs_file_delete(const char* path);

int lfs_open(lfs_file_t* file, const char* path, int flags);

int lfs_close(lfs_file_t* file);

int lfs_read(lfs_file_t* file, uint8_t* buff, uint32_t len, uint32_t* read_len);

int lfs_write(lfs_file_t* file, uint8_t* data, uint32_t len, uint32_t* write_len);

lfs_t* lfs_get_fs(void);

int lfs_state(const char* path, struct lfs_info * info);

int lfs_show(char* path, uint8_t root_path);

int lfs_rename_file(const char* old_path, const char* new_path);

int lfs_sync(lfs_file_t* file);

int lfs_lseek(lfs_file_t* file, uint32_t offset, int whence);

int lfs_tell(lfs_file_t* file);

int lfs_size(lfs_file_t* file);

// used to visualize file system structure
typedef struct {
    char name[MAX_FILE_PATH];
    char path[MAX_FILE_PATH];
    uint32_t size;
    uint8_t type;
    uint8_t depth;
} lfs_file_info_t;

typedef struct {
    lfs_file_info_t files[MAX_FILE_COUNT];
    uint16_t count;
} lfs_scan_result_t;

int lfs_scan_and_store(const char* path, lfs_scan_result_t* result);


#ifdef __cplusplus
}
#endif

#endif

