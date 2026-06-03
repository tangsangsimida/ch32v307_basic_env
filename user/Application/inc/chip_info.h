/**
 * @file chip_info.h
 * @brief 芯片信息接口定义
 */

#ifndef CHIP_INFO_H
#define CHIP_INFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UID长度定义
 */
#define CHIP_UID_LENGTH     12  /* 96位 = 12字节 */

/**
 * @brief 读取96位芯片唯一ID
 * @param uid 存储UID的数组（至少12字节）
 */
void chip_get_uid(uint8_t *uid);

/**
 * @brief 打印芯片信息
 */
void chip_print_info(void);

/**
 * @brief 打印UID
 * @param uid UID数组
 */
void chip_print_uid(const uint8_t *uid);

#ifdef __cplusplus
}
#endif

#endif /* CHIP_INFO_H */
