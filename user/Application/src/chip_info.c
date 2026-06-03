/**
 * @file chip_info.c
 * @brief 芯片信息功能实现
 */

#include "chip_info.h"
#include "hal.h"
#include <stdio.h>

/**
 * @brief UID地址定义
 *
 * CH32V307的96位唯一ID存储在固定地址
 * 地址: 0x1FFFF7E8 - 0x1FFFF7F3 (12字节)
 */
#define UID_BASE_ADDR   0x1FFFF7E8

void chip_get_uid(uint8_t *uid)
{
    const uint8_t *uid_ptr = (const uint8_t *)UID_BASE_ADDR;

    for (int i = 0; i < CHIP_UID_LENGTH; i++)
    {
        uid[i] = uid_ptr[i];
    }
}

void chip_print_info(void)
{
    printf("SystemClk: %ld Hz\r\n", hal_system_get_clock());
    printf("ChipID:    %08lX\r\n", hal_get_chip_id());
}

void chip_print_uid(const uint8_t *uid)
{
    /* 原始十六进制 */
    printf("  Hex: ");
    for (int i = 0; i < CHIP_UID_LENGTH; i++)
    {
        printf("%02X", uid[i]);
    }
    printf("\r\n");

    /* 32位分组显示 */
    printf("\r\nFormatted:\r\n");
    printf("  UID[31:0]:   %02X%02X%02X%02X\r\n", uid[3], uid[2], uid[1], uid[0]);
    printf("  UID[63:32]:  %02X%02X%02X%02X\r\n", uid[7], uid[6], uid[5], uid[4]);
    printf("  UID[95:64]:  %02X%02X%02X%02X\r\n", uid[11], uid[10], uid[9], uid[8]);
}
