/**
 * @file main.c
 * @brief LED闪烁 + 串口输出 + 以太网 + USB CDC + DHCP 示例
 */

#include "hal.h"
#include "led.h"
#include "chip_info.h"
#include "eth_demo.h"
#include "usb_cdc_demo.h"
#include "dhcp_client.h"
#include "debug.h"
#include <stdio.h>

int main(void)
{
    uint8_t uid[CHIP_UID_LENGTH];
    uint8_t usb_rx_buf[64];
    uint16_t usb_rx_len;
    dhcp_info_t net_info;
    char ip_str[16];

    /* 初始化HAL */
    hal_init();

    /* 初始化LED */
    led_init();

    /* 打印标题 */
    printf("\r\n========================================\r\n");
    printf("  CH32V307 综合示例\r\n");
    printf("========================================\r\n\r\n");

    /* 打印芯片信息 */
    chip_print_info();

    /* 获取并打印UID */
    chip_get_uid(uid);
    printf("\r\n96-bit Unique ID:\r\n");
    chip_print_uid(uid);

    /* 初始化以太网 */
    printf("\r\n--- Ethernet ---\r\n");
    eth_demo_init();

    /* 初始化 DHCP */
    printf("\r\n--- DHCP ---\r\n");
    dhcp_init();

    /* 初始化 USB CDC */
    printf("\r\n--- USB CDC ---\r\n");
    usb_cdc_demo_init();

    printf("\r\n========================================\r\n");

    /* 主循环 */
    while (1)
    {
        /* LED 交替闪烁 */
        led_toggle(LED1);
        led_toggle(LED2);

        /* 以太网轮询 */
        eth_demo_poll();

        /* DHCP 处理 */
        dhcp_poll();

        /* 检查是否获取到 IP */
        if (dhcp_get_state() == DHCP_STATE_BOUND)
        {
            if (dhcp_get_info(&net_info) == 0)
            {
                dhcp_ip_to_str(net_info.ip, ip_str);
                /* 只打印一次 */
                static uint8_t printed = 0;
                if (!printed)
                {
                    printf("\r\n=== Network Ready ===\r\n");
                    printf("IP:      %s\r\n", ip_str);
                    dhcp_ip_to_str(net_info.mask, ip_str);
                    printf("Mask:    %s\r\n", ip_str);
                    dhcp_ip_to_str(net_info.gateway, ip_str);
                    printf("Gateway: %s\r\n", ip_str);
                    dhcp_ip_to_str(net_info.dns, ip_str);
                    printf("DNS:     %s\r\n", ip_str);
                    printf("=====================\r\n\r\n");
                    printed = 1;
                }
            }
        }

        /* USB CDC 回显 */
        if (usb_cdc_is_enumerated())
        {
            usb_rx_len = usb_cdc_read(usb_rx_buf, sizeof(usb_rx_buf));
            if (usb_rx_len > 0)
            {
                usb_cdc_send(usb_rx_buf, usb_rx_len);
            }
        }

        /* 主循环延时，控制 LED 闪烁速率和轮询频率 */
        Delay_Ms(10);
    }
}
