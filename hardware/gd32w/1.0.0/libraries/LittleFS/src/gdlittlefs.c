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

#include "gdlittlefs.h"

static lfs_t* p_lfs = NULL;
static struct lfs_config* p_lfs_cfg = NULL;

static int block_device_read(const struct lfs_config *c, lfs_block_t block,
                              lfs_off_t off, void *buffer, lfs_size_t size)
{
    if(raw_flash_read(block * SIP_FLASH_SECTOR_SIZE + SIP_FALSH_START_ADDR + off,buffer,size)){
        printf("block_device_read error\n");
        return LFS_ERR_CORRUPT;
    }

    return LFS_ERR_OK;
}

static int block_device_prog(const struct lfs_config *c, lfs_block_t block,
                              lfs_off_t off, const void *buffer, lfs_size_t size)
{
    if(raw_flash_write(block * SIP_FLASH_SECTOR_SIZE + SIP_FALSH_START_ADDR + off,buffer,size)){
        printf("block_device_prog error\n");
        return LFS_ERR_CORRUPT;
    }

    return LFS_ERR_OK;
}

static int block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
    if(raw_flash_erase(block * SIP_FLASH_SECTOR_SIZE + SIP_FALSH_START_ADDR,SIP_FLASH_SECTOR_SIZE)){
        printf("block_device_erase error\n");
        return LFS_ERR_CORRUPT;
    }

    return LFS_ERR_OK;
}

static int block_device_sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}


bool lfs_mk_mount(void)
{
    int err;
    p_lfs = sys_malloc(sizeof(lfs_t));
    if(p_lfs == NULL){
        sys_mfree(p_lfs);
        return false;
    }

    p_lfs_cfg = sys_malloc(sizeof(struct lfs_config));
    if(p_lfs_cfg == NULL){
        sys_mfree(p_lfs);
        return false;
    }

    sys_memset(p_lfs_cfg, 0, sizeof(struct lfs_config));
    sys_memset(p_lfs, 0, sizeof(lfs_t));


    p_lfs_cfg->read = block_device_read;
    p_lfs_cfg->prog = block_device_prog;
    p_lfs_cfg->erase = block_device_erase;
    p_lfs_cfg->sync = block_device_sync;

    p_lfs_cfg->read_size = SIP_FLASH_SECTOR_SIZE;
    p_lfs_cfg->prog_size = SIP_FLASH_SECTOR_SIZE;

    p_lfs_cfg->block_size = SIP_FLASH_SECTOR_SIZE;
    p_lfs_cfg->block_count = SIP_FULL_MEM_SIZE/ SIP_FLASH_SECTOR_SIZE;
    p_lfs_cfg->cache_size = SIP_FLASH_SECTOR_SIZE;
    p_lfs_cfg->lookahead_size = SIP_FLASH_SECTOR_SIZE;
    p_lfs_cfg->block_cycles = 500;

    err = lfs_mount(p_lfs, p_lfs_cfg);

    if(err){
        printf("lfs_mount error: %d\n", err);
        lfs_format(p_lfs, p_lfs_cfg);
        err = lfs_mount(p_lfs, p_lfs_cfg);
    }

    if(err){
        printf("lfs_mount error: %d\n", err);
        sys_mfree(p_lfs);
        sys_mfree(p_lfs_cfg);
        p_lfs = NULL;
        p_lfs_cfg = NULL;
        return false;
    }
    return true;
}

bool lfs_mk_unmount(void){
    int err = lfs_unmount(p_lfs);
    if(err) return false;

    sys_mfree(p_lfs);
    sys_mfree(p_lfs_cfg);
    p_lfs = NULL;
    p_lfs_cfg = NULL;
    return true;
}

int lfs_mk_dir(const char* path){
    if(p_lfs == NULL || path == NULL){
        return LFS_ERR_IO;
    }

    return lfs_mkdir(p_lfs, path);
}

int lfs_open(lfs_file_t* file, const char* path, int flags){
    if(p_lfs == NULL || path == NULL){
        return LFS_ERR_IO;
    }
    struct lfs_info info;
    lfs_state(path, &info);
    if(info.type == LFS_TYPE_DIR){
        // lfs_dir_open
        return LFS_ERR_ISDIR;
    }

    return lfs_file_open(p_lfs, file, path, flags);
}

int lfs_close(lfs_file_t* file){
    if(p_lfs == NULL || file == NULL){
        return LFS_ERR_IO;
    }

    return lfs_file_close(p_lfs, file);
}

int lfs_read(lfs_file_t* file, uint8_t* buff, uint32_t len, uint32_t* read_len){
    int res = LFS_ERR_OK;

    if(p_lfs == NULL || file == NULL || buff == NULL){
        return LFS_ERR_IO;
    }

    res = lfs_file_read(p_lfs, file, buff, len);

    if(res < LFS_ERR_OK){
        return res;
    }

    if(read_len){
        *read_len = res;
    }

    return LFS_ERR_OK;
}

int lfs_sync(lfs_file_t* file){
    if(p_lfs == NULL || file == NULL){
        return LFS_ERR_IO;
    }
    return lfs_file_sync(p_lfs, file);
}

int lfs_lseek(lfs_file_t* file, uint32_t offset, int whence){
    if(p_lfs == NULL || file == NULL){
        return LFS_ERR_IO;
    }

    return lfs_file_seek(p_lfs, file, offset, whence);
}

int lfs_tell(lfs_file_t* file){
    if(p_lfs == NULL || file == NULL){
        return LFS_ERR_IO;
    }

    return lfs_file_tell(p_lfs, file);
}

int lfs_size(lfs_file_t* file){
    if(p_lfs == NULL || file == NULL){
        return LFS_ERR_IO;
    }

    return lfs_file_size(p_lfs, file);
}

int lfs_write(lfs_file_t* file, uint8_t* data, uint32_t len, uint32_t* write_len){

    int res = LFS_ERR_OK;

    if(p_lfs == NULL || file == NULL || data == NULL){
        return LFS_ERR_IO;
    }

    res = lfs_file_write(p_lfs, file, data, len);

    if(res < LFS_ERR_OK){
        return res;
    }

    if(write_len){
        *write_len = res;
    }
    return LFS_ERR_OK;
}

lfs_t* lfs_get_fs(void){
    return p_lfs;
}

int lfs_state(const char* path, struct lfs_info * info){

    if(p_lfs == NULL || path == NULL){
        return LFS_ERR_IO;
    }

    if(info == NULL){
        struct lfs_info temp_info;
        return lfs_stat(p_lfs, path, &temp_info);
    }

    return lfs_stat(p_lfs, path, info);
}

int lfs_rename_file(const char* old_path, const char* new_path){
    if(p_lfs == NULL || old_path == NULL || new_path == NULL){
        return LFS_ERR_IO;
    }
    return lfs_rename(p_lfs, old_path, new_path);
}

int lfs_rm_dir(const char* path){
    return lfs_rm_file(path);
}
int lfs_rm_file(const char* path) {
    if (p_lfs == NULL || path == NULL) {
        return LFS_ERR_IO;
    }

    struct lfs_info info;
    int err = lfs_stat(p_lfs, path, &info);
    if (err != LFS_ERR_OK) {
        return err;
    }

    if (info.type == LFS_TYPE_REG) {
        return lfs_remove(p_lfs, path);
    }

    if (info.type == LFS_TYPE_DIR) {
        lfs_dir_t dir;
        err = lfs_dir_open(p_lfs, &dir, path);
        if (err != LFS_ERR_OK) {
            return err;
        }
        while (true) {
            struct lfs_info entry;
            int res = lfs_dir_read(p_lfs, &dir, &entry);

            if (res <= 0) {
                break;
            }

            if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0) {
                continue;
            }

            char full_path[256];
            int path_len = strlen(path);
            int name_len = strlen(entry.name);

            if (path_len + name_len + 2 > sizeof(full_path)) {
                lfs_dir_close(p_lfs, &dir);
                return LFS_ERR_NAMETOOLONG;
            }

            strcpy(full_path, path);
            if (full_path[path_len - 1] != '/') {
                strcat(full_path, "/");
            }
            strcat(full_path, entry.name);

            int rm_err = lfs_rm_file(full_path);
            if (rm_err != LFS_ERR_OK) {
                lfs_dir_close(p_lfs, &dir);
                return rm_err;
            }
        }

        lfs_dir_close(p_lfs, &dir);

        if(strcmp(path, "/") == 0){
            return LFS_ERR_OK;
        }

        return lfs_remove(p_lfs, path);
    }

    return LFS_ERR_IO;
}

int lfs_file_delete(const char* path){
    int res = LFS_ERR_OK;
    struct lfs_info info;

    if(p_lfs == NULL || path == NULL){
        return LFS_ERR_IO;
    }

    res = lfs_stat(p_lfs, path, &info);
    if(res == LFS_ERR_OK){
        res = lfs_remove(p_lfs, path);
    }

    return res;
}

static int lfs_scan_dir(char* path){
    int res = LFS_ERR_OK;
    lfs_dir_t dir;
    uint32_t i;
    static struct lfs_info info;

    res = lfs_dir_open(p_lfs, &dir, path);
    if(res < LFS_ERR_OK){
        printf("lfs_dir_open error\n");
        return res;
    }


    for(;;){
        res = lfs_dir_read(p_lfs, &dir, &info);
        if(res < LFS_ERR_OK){
            printf("lfs_dir_read error\n");
            break;
        }

        if(res == 0){
            break;
        }

        if(info.type == LFS_TYPE_DIR){
            printf("DIR %s/%s\n", path, info.name);
            i = strlen(path);
            snprintf(&path[i], (MAX_FILE_PATH+2-i), "/%s", info.name);
            res = lfs_scan_dir(path);
            if(res < LFS_ERR_OK){
                printf("lfs_scan_dir error\n");
                break;
            }
        }else{
            printf("FILE %s/%s %d\n", path, info.name, info.size);
        }
    }

    res = lfs_dir_close(p_lfs, &dir);
    return res;

}

int lfs_show(char* path, uint8_t root_path){
    int res = LFS_ERR_OK;

    char path_buffer[MAX_FILE_PATH + 1];

    if(p_lfs == NULL){
        return -1;
    }

    if(root_path){
        strncpy(path_buffer, "/", MAX_FILE_PATH);
    }else{
        strncpy(path_buffer, path, MAX_FILE_PATH);
    }

    res = lfs_scan_dir(path_buffer);
    return res;
}

static int lfs_scan_dir_recursive(const char* path, lfs_scan_result_t* result, uint8_t depth) {
    lfs_dir_t dir;
    int err;

    if (p_lfs == NULL) {
        return LFS_ERR_IO;
    }

    if (result->count >= MAX_FILE_COUNT) {
        return LFS_ERR_NOSPC;
    }

    err = lfs_dir_open(p_lfs, &dir, path);
    if (err != LFS_ERR_OK) {
        return err;
    }

    while (true) {
        struct lfs_info info;
        int res = lfs_dir_read(p_lfs, &dir, &info);

        if (res < 0) {
            lfs_dir_close(p_lfs, &dir);
            return res;
        }

        if (res == 0) {
            break;
        }

        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
            continue;
        }

        if (result->count >= MAX_FILE_COUNT) {
            lfs_dir_close(p_lfs, &dir);
            return LFS_ERR_NOSPC;
        }

        char fullPath[MAX_FILE_PATH];
        int pathLen = strlen(path);
        int nameLen = strlen(info.name);

        if (pathLen + nameLen + 2 > MAX_FILE_PATH) {
            continue;
        }

        strcpy(fullPath, path);
        if (pathLen > 0 && fullPath[pathLen - 1] != '/') {
            strcat(fullPath, "/");
        }
        strcat(fullPath, info.name);

        lfs_file_info_t* fileInfo = &result->files[result->count];
        strncpy(fileInfo->name, info.name, MAX_FILE_PATH - 1);
        fileInfo->name[MAX_FILE_PATH - 1] = '\0';
        strncpy(fileInfo->path, fullPath, MAX_FILE_PATH - 1);
        fileInfo->path[MAX_FILE_PATH - 1] = '\0';
        fileInfo->size = info.size;
        fileInfo->type = info.type;
        fileInfo->depth = depth;
        result->count++;

        if (info.type == LFS_TYPE_DIR) {
            int subErr = lfs_scan_dir_recursive(fullPath, result, depth + 1);
            if (subErr != LFS_ERR_OK && subErr != LFS_ERR_NOSPC) {
                lfs_dir_close(p_lfs, &dir);
                return subErr;
            }
        }
    }

    lfs_dir_close(p_lfs, &dir);
    return LFS_ERR_OK;
}

int lfs_scan_and_store(const char* path, lfs_scan_result_t* result) {
    if (p_lfs == NULL || result == NULL) {
        return LFS_ERR_IO;
    }

    result->count = 0;
    memset(result->files, 0, sizeof(result->files));

    const char* scanPath = (path == NULL) ? "/" : path;

    int err = lfs_scan_dir_recursive(scanPath, result, 0);
    if (err != LFS_ERR_OK && err != LFS_ERR_NOSPC) {
        return err;
    }

    return result->count;
}
