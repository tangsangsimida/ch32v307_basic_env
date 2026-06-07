/**
 * @file debug_config.h
 * @brief 集中式调试日志配置
 *
 * 用法：
 *   1. 在本文件中设置各模块的调试等级（DBG_LVL_OFF / ERR / WARN / INFO）
 *   2. 在源文件中 #include "debug_config.h"，使用模块宏输出日志：
 *        DHCP_DBG_INFO("ACK: IP=%s", ip_str);
 *        NET_DBG_WARN("unknown protocol 0x%02X", proto);
 *        ETH_DBG_ERR("DMA overflow");
 *
 * 等级说明：
 *   DBG_LVL_OFF  — 关闭该模块所有日志
 *   DBG_LVL_ERR  — 仅错误
 *   DBG_LVL_WARN — 错误 + 警告
 *   DBG_LVL_INFO — 错误 + 警告 + 信息
 *
 * 全局开关：
 *   DEBUG_ENABLE — 定义则启用全部模块日志（等级由各模块等级控制）
 *                  未定义则全部静默（Release 模式）
 */

#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#include <stdio.h>

/* ========================================================================
 * 全局开关
 *
 * Debug 构建自动启用；Release 构建（-DNDEBUG）全部静默。
 * 也可手动注释/取消注释下一行来强制控制。
 * ======================================================================== */
#ifndef NDEBUG
#define DEBUG_ENABLE
#endif

/* ========================================================================
 * 调试等级定义
 * ======================================================================== */
#define DBG_LVL_OFF  0
#define DBG_LVL_ERR  1
#define DBG_LVL_WARN 2
#define DBG_LVL_INFO 3

/* ========================================================================
 * 模块等级配置
 *
 * 修改此处即可控制各模块日志输出粒度。
 * ======================================================================== */
#define DHCP_DBG_LVL   DBG_LVL_INFO   /* DHCP 状态转换：OFFER/ACK/NAK */
#define NET_DBG_LVL    DBG_LVL_WARN   /* ARP/ICMP/ECHO：仅警告和错误 */
#define ETH_DBG_LVL    DBG_LVL_WARN   /* 以太网驱动：仅警告和错误 */

/* ========================================================================
 * DHCP 模块日志宏
 * ======================================================================== */
#ifdef DEBUG_ENABLE
#if DHCP_DBG_LVL >= DBG_LVL_ERR
#define DHCP_DBG_ERR(fmt, ...)   printf("[DHCP][ERR] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DHCP_DBG_ERR(fmt, ...)   ((void)0)
#endif
#if DHCP_DBG_LVL >= DBG_LVL_WARN
#define DHCP_DBG_WARN(fmt, ...)  printf("[DHCP][WARN] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DHCP_DBG_WARN(fmt, ...)  ((void)0)
#endif
#if DHCP_DBG_LVL >= DBG_LVL_INFO
#define DHCP_DBG_INFO(fmt, ...)  printf("[DHCP] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DHCP_DBG_INFO(fmt, ...)  ((void)0)
#endif
#else /* !DEBUG_ENABLE */
#define DHCP_DBG_ERR(fmt, ...)   ((void)0)
#define DHCP_DBG_WARN(fmt, ...)  ((void)0)
#define DHCP_DBG_INFO(fmt, ...)  ((void)0)
#endif

/* ========================================================================
 * NET 模块日志宏（ARP / ICMP / UDP Echo）
 * ======================================================================== */
#ifdef DEBUG_ENABLE
#if NET_DBG_LVL >= DBG_LVL_ERR
#define NET_DBG_ERR(fmt, ...)    printf("[NET][ERR] " fmt "\r\n", ##__VA_ARGS__)
#else
#define NET_DBG_ERR(fmt, ...)    ((void)0)
#endif
#if NET_DBG_LVL >= DBG_LVL_WARN
#define NET_DBG_WARN(fmt, ...)   printf("[NET][WARN] " fmt "\r\n", ##__VA_ARGS__)
#else
#define NET_DBG_WARN(fmt, ...)   ((void)0)
#endif
#if NET_DBG_LVL >= DBG_LVL_INFO
#define NET_DBG_INFO(fmt, ...)   printf("[NET] " fmt "\r\n", ##__VA_ARGS__)
#else
#define NET_DBG_INFO(fmt, ...)   ((void)0)
#endif
#else /* !DEBUG_ENABLE */
#define NET_DBG_ERR(fmt, ...)    ((void)0)
#define NET_DBG_WARN(fmt, ...)   ((void)0)
#define NET_DBG_INFO(fmt, ...)   ((void)0)
#endif

/* ========================================================================
 * ETH 模块日志宏
 * ======================================================================== */
#ifdef DEBUG_ENABLE
#if ETH_DBG_LVL >= DBG_LVL_ERR
#define ETH_DBG_ERR(fmt, ...)    printf("[ETH][ERR] " fmt "\r\n", ##__VA_ARGS__)
#else
#define ETH_DBG_ERR(fmt, ...)    ((void)0)
#endif
#if ETH_DBG_LVL >= DBG_LVL_WARN
#define ETH_DBG_WARN(fmt, ...)   printf("[ETH][WARN] " fmt "\r\n", ##__VA_ARGS__)
#else
#define ETH_DBG_WARN(fmt, ...)   ((void)0)
#endif
#if ETH_DBG_LVL >= DBG_LVL_INFO
#define ETH_DBG_INFO(fmt, ...)   printf("[ETH] " fmt "\r\n", ##__VA_ARGS__)
#else
#define ETH_DBG_INFO(fmt, ...)   ((void)0)
#endif
#else /* !DEBUG_ENABLE */
#define ETH_DBG_ERR(fmt, ...)    ((void)0)
#define ETH_DBG_WARN(fmt, ...)   ((void)0)
#define ETH_DBG_INFO(fmt, ...)   ((void)0)
#endif

#endif /* DEBUG_CONFIG_H */
