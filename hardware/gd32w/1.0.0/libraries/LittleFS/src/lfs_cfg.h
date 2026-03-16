#ifndef LFS_CFG_H
#define LFS_CFG_H

#include "wrapper_os.h"

// #define  LFS_THREADSAFE

#define LFS_NAME_MAX 32

#define LFS_MALLOC sys_malloc
#define LFS_FREE   sys_mfree

#endif


