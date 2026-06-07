/**
 * @file net_protocols.h
 * @brief 最小化网络协议栈接口
 *
 * 实现 ARP + IP + ICMP + UDP + DHCP 协议。
 * 通过 eth_demo 提供的原始帧收发能力工作。
 */

#ifndef NET_PROTOCOLS_H
#define NET_PROTOCOLS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DHCP 状态
 */
typedef enum {
    DHCP_STATE_IDLE = 0,
    DHCP_STATE_DISCOVER,
    DHCP_STATE_OFFER,
    DHCP_STATE_REQUEST,
    DHCP_STATE_BOUND,
    DHCP_STATE_ERROR
} dhcp_state_t;

/**
 * @brief 网络信息
 */
typedef struct {
    uint32_t ip;        /* 本机 IP */
    uint32_t mask;      /* 子网掩码 */
    uint32_t gateway;   /* 网关 */
    uint32_t dns;       /* DNS 服务器 */
    uint32_t server;    /* DHCP 服务器 */
    uint32_t lease;     /* 租约时间（秒） */
} dhcp_info_t;

/**
 * @brief 初始化网络协议栈
 *
 * 自动使用 ETH demo 的 MAC 地址，启动 DHCP 状态机。
 */
void dhcp_init(void);

/**
 * @brief 设置静态 IP（跳过 DHCP，用于调试）
 */
void dhcp_set_static_ip(uint32_t ip, uint32_t mask, uint32_t gw, uint32_t dns);

/**
 * @brief 网络协议主任务（需在主循环中调用）
 *
 * 接收以太网帧并分发到 ARP/ICMP/UDP/DHCP 处理。
 * DHCP 状态机：DISCOVER → OFFER → REQUEST → BOUND。
 */
void dhcp_poll(void);

/**
 * @brief 获取 DHCP 状态
 */
dhcp_state_t dhcp_get_state(void);

/**
 * @brief 获取网络信息
 * @param info 输出参数
 * @return 0 成功（已获取 IP），-1 未就绪
 */
int dhcp_get_info(dhcp_info_t *info);

/**
 * @brief 将 IP 地址转为点分十进制字符串
 *
 * @note IP 地址以网络字节序存储在 uint32_t 中（大端），
 *       即 ip 的最高字节对应点分十进制的第一段。
 *
 * @param ip IP 地址（网络字节序，如 0xC0A80101 表示 192.168.1.1）
 * @param buf 输出缓冲区（至少 16 字节，包含末尾 '\0'）
 */
void dhcp_ip_to_str(uint32_t ip, char *buf);

/**
 * @brief 发送 UDP 数据包
 *
 * 自动构造以太网帧 + IP 头 + UDP 头，调用 eth_demo_send 发送。
 * 单播时使用 ARP 缓存的 MAC 地址；缓存未命中时回退为广播。
 *
 * @param dst_ip   目标 IP（网络字节序）
 * @param dst_port 目标端口（主机字节序）
 * @param src_port 源端口（主机字节序）
 * @param data     UDP 负载数据
 * @param data_len 负载长度
 */
void udp_send(uint32_t dst_ip, uint16_t dst_port,
              uint16_t src_port, const uint8_t *data, uint16_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* NET_PROTOCOLS_H */
