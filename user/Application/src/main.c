/**
 * @file main.c
 * @brief 获取96位芯片唯一ID示例
 */

#include "hal.h"
#include "chip_info.h"
#include <stdio.h>

int main(void)
{
    uint8_t uid[CHIP_UID_LENGTH];

    /* 初始化HAL */
    hal_init();

    /* 打印标题 */
    printf("\r\n========================================\r\n");
    printf("  CH32V307 芯片唯一ID读取示例\r\n");
    printf("========================================\r\n\r\n");

    /* 打印芯片信息 */
    chip_print_info();

    /* 获取并打印UID */
    chip_get_uid(uid);

    printf("\r\n96-bit Unique ID:\r\n");
    chip_print_uid(uid);

    printf("\r\n========================================\r\n");

    /* 主循环 */
    while (1)
    {
        hal_delay_ms(1000);
    }
}
