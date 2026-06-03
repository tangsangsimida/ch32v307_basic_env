/**
 * @file syscalls.c
 * @brief 空的系统调用实现（消除newlib链接警告）
 *
 * 嵌入式系统不需要这些系统调用，提供空实现即可。
 */

#include "ch32v30x.h"
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

/**
 * @brief 关闭文件（空实现）
 */
int _close(int file)
{
    (void)file;
    return -1;
}

/**
 * @brief 获取文件状态（空实现）
 */
int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

/**
 * @brief 检查是否为终端（空实现）
 */
int _isatty(int file)
{
    (void)file;
    return 1;
}

/**
 * @brief 定位文件（空实现）
 */
int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

/**
 * @brief 读取文件（空实现）
 */
int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

/**
 * @brief 写入文件（由debug.c实现）
 */

/**
 * @brief 堆内存分配（由debug.c实现）
 */
