/**
 * @file eth_demo.h
 * @brief 以太网 MAC_RAW Demo 接口定义
 *
 * 基于 CH32V307 内部 MAC + 外部 PHY (RMII)，不依赖协议栈，
 * 直接操作 DMA 描述符收发原始以太网帧。
 */

#ifndef ETH_DEMO_H
#define ETH_DEMO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 以太网链路状态
 */
typedef enum {
    ETH_LINK_DOWN = 0,
    ETH_LINK_UP   = 1
} eth_link_status_t;

/**
 * @brief 初始化以太网外设（RMII 模式）
 *
 * 配置 RMII GPIO、MAC 寄存器、DMA 描述符链、PHY。
 * MAC 地址从芯片 ROM 自动读取。
 */
void eth_demo_init(void);

/**
 * @brief 以太网主任务（需在主循环中调用）
 *
 * 处理链路状态变化、PHY 重协商等。
 */
void eth_demo_poll(void);

/**
 * @brief 发送原始以太网帧
 *
 * @note 非线程安全，不可在中断上下文中调用。
 *       ETH_IRQHandler 可能同时操作 DMA 描述符。
 *
 * @param pbuf 帧数据指针
 * @param len  帧长度（不超过 ETH_MAX_PACKET_SIZE，通常 1524）
 * @return 1 成功，0 失败（与底层 ETH_HandleTxPkt 一致）
 */
uint32_t eth_demo_send(uint8_t *pbuf, uint16_t len);

/**
 * @brief 接收原始以太网帧
 *
 * @note 非线程安全，不可在中断上下文中调用。
 *
 * @param pbuf 接收缓冲区（至少 ETH_MAX_PACKET_SIZE 字节）
 * @return 接收到的帧长度，0 表示无数据
 */
uint32_t eth_demo_recv(uint8_t *pbuf);

/**
 * @brief 获取当前链路状态
 * @return ETH_LINK_UP 或 ETH_LINK_DOWN
 */
eth_link_status_t eth_demo_get_link_status(void);

/**
 * @brief 获取 MAC 地址
 * @param mac 6 字节缓冲区（调用者确保至少 6 字节）
 */
void eth_demo_get_mac_addr(uint8_t *mac);

#ifdef __cplusplus
}
#endif

#endif /* ETH_DEMO_H */
