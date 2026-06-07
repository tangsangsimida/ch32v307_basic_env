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
    uint8_t network_info_printed = 0;

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

    /* 启动 DHCP 自动获取 IP */
    printf("\r\n--- Network ---\r\n");
    dhcp_init();

    /* 初始化 USB CDC */
    printf("\r\n--- USB CDC ---\r\n");
    usb_cdc_demo_init();

    printf("\r\n========================================\r\n");

    printf("Network:    DHCP auto IP\r\n");
    printf("UDP Echo:   port 7 after DHCP bound\r\n");
    printf("========================================\r\n");

    /* 主循环 */
    while (1)
    {
        /* LED 交替闪烁 */
        led_toggle(LED1);
        led_toggle(LED2);

        /* 以太网轮询（链路状态 + 收发帧） */
        eth_demo_poll();

        /* 网络处理（ARP 应答 + UDP Echo） */
        dhcp_poll();

        if (!network_info_printed && dhcp_get_state() == DHCP_STATE_BOUND)
        {
            dhcp_info_t info;
            if (dhcp_get_info(&info) == 0)
            {
                char ip_str[16];

                printf("\r\n--- DHCP Bound ---\r\n");
                printf("IP:         "); dhcp_ip_to_str(info.ip, ip_str);      printf("%s\r\n", ip_str);
                printf("Mask:       "); dhcp_ip_to_str(info.mask, ip_str);    printf("%s\r\n", ip_str);
                printf("Gateway:    "); dhcp_ip_to_str(info.gateway, ip_str); printf("%s\r\n", ip_str);
                printf("DNS:        "); dhcp_ip_to_str(info.dns, ip_str);     printf("%s\r\n", ip_str);
                printf("------------------\r\n");
                network_info_printed = 1;
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
