/**
 * @file main.c
 * @brief LED交错闪烁 + 串口输出示例
 *
 * LED1(PE8)和LED2(PE9)交替亮灭，间隔500ms。
 * 串口每500ms输出一次"helloworld"。
 */

#include "hal.h"
#include "led.h"
#include "chip_info.h"
#include <stdio.h>

int main(void)
{
    uint8_t uid[CHIP_UID_LENGTH];

    /* 初始化HAL */
    hal_init();

    /* 初始化LED */
    led_init();

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
        led_toggle(LED1);
        led_toggle(LED2);
        printf("helloworld\r\n");
        hal_delay_ms(500);
    }
}
