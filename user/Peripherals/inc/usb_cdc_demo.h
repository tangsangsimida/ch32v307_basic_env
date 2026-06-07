/**
 * @file usb_cdc_demo.h
 * @brief USB CDC (Virtual COM Port) Demo 接口定义
 *
 * 使用 USBFS 控制器 (PA11/PA12)，Full-Speed 模式。
 * 枚举为 CDC 虚拟串口，收到数据后回显。
 */

#ifndef USB_CDC_DEMO_H
#define USB_CDC_DEMO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 USB CDC 设备
 *
 * 配置 USBFS 时钟、端点、中断，使能 USB 设备。
 * PC 会识别到 Virtual COM Port 设备。
 */
void usb_cdc_demo_init(void);

/**
 * @brief 检查 USB 是否已枚举完成
 * @return 1 已枚举，0 未枚举
 */
uint8_t usb_cdc_is_enumerated(void);

/**
 * @brief 通过 USB CDC 发送数据
 *
 * @note 超过 64 字节的数据会被静默截断到 64 字节。
 *
 * @param data 数据指针
 * @param len  数据长度（超过 64 字节时截断）
 * @return 0 成功，其他失败
 */
uint8_t usb_cdc_send(const uint8_t *data, uint16_t len);

/**
 * @brief 检查是否有数据可读
 * @return 可读数据长度，0 表示无数据
 */
uint16_t usb_cdc_available(void);

/**
 * @brief 读取接收到的数据
 * @param buf 接收缓冲区
 * @param max_len 缓冲区最大长度
 * @return 实际读取的字节数
 */
uint16_t usb_cdc_read(uint8_t *buf, uint16_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* USB_CDC_DEMO_H */
