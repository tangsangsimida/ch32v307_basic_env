/**
 * @file dhcp_client.c
 * @brief 最小化 DHCP 客户端实现
 *
 * 实现 ARP + IP + UDP + DHCP 协议栈。
 * 通过 eth_demo 的 send/recv 收发原始以太网帧。
 */

#include "dhcp_client.h"
#include "eth_demo.h"
#include "ch32v30x.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

#define DHCP_DEBUG /* DHCP 调试输出 */

/* ========================================================================
 * 协议常量
 * ======================================================================== */

/* 以太网帧类型 */
#define ETH_TYPE_ARP        0x0806
#define ETH_TYPE_IP         0x0800

/* ARP */
#define ARP_HTYPE_ETH       0x0001
#define ARP_PTYPE_IP        0x0800
#define ARP_OP_REQUEST      0x0001
#define ARP_OP_REPLY        0x0002

/* IP */
#define IP_VER_IHL          0x45
#define IP_PROTO_UDP        17
#define IP_TTL              128

/* UDP */
#define UDP_PORT_BOOTPC     68
#define UDP_PORT_BOOTPS     67

/* DHCP */
#define DHCP_MAGIC          0x63825363
#define DHCP_OP_BOOTREQUEST 1
#define DHCP_OP_BOOTREPLY   2
#define DHCP_HTYPE_ETH      1
#define DHCP_HLEN_ETH       6

/* DHCP 消息类型 */
#define DHCP_DISCOVER       1
#define DHCP_OFFER          2
#define DHCP_REQUEST        3
#define DHCP_ACK            5
#define DHCP_NAK            6

/* DHCP 选项 */
#define OPT_PAD             0
#define OPT_SUBNET_MASK     1
#define OPT_ROUTER          3
#define OPT_DNS             6
#define OPT_REQUESTED_IP    50
#define OPT_LEASE_TIME      51
#define OPT_MSG_TYPE        53
#define OPT_SERVER_ID       54
#define OPT_END             255

/* ========================================================================
 * 协议结构体（packed）
 * ======================================================================== */

#pragma pack(push, 1)

/* 以太网帧头 */
typedef struct {
    uint8_t  dst[6];
    uint8_t  src[6];
    uint16_t type;
} eth_hdr_t;

/* ARP 包 */
typedef struct {
    eth_hdr_t eth;
    uint16_t  htype;
    uint16_t  ptype;
    uint8_t   hlen;
    uint8_t   plen;
    uint16_t  oper;
    uint8_t   sha[6];   /* 发送方 MAC */
    uint8_t   spa[4];   /* 发送方 IP */
    uint8_t   tha[6];   /* 目标 MAC */
    uint8_t   tpa[4];   /* 目标 IP */
} arp_pkt_t;

/* IP 头 */
typedef struct {
    uint8_t   ver_ihl;
    uint8_t   tos;
    uint16_t  total_len;
    uint16_t  id;
    uint16_t  flags_offset;
    uint8_t   ttl;
    uint8_t   protocol;
    uint16_t  checksum;
    uint32_t  src_ip;
    uint32_t  dst_ip;
} ip_hdr_t;

/* UDP 头 */
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} udp_hdr_t;

/* DHCP 包 */
typedef struct {
    uint8_t   op;
    uint8_t   htype;
    uint8_t   hlen;
    uint8_t   hops;
    uint32_t  xid;
    uint16_t  secs;
    uint16_t  flags;
    uint32_t  ciaddr;
    uint32_t  yiaddr;
    uint32_t  siaddr;
    uint32_t  giaddr;
    uint8_t   chaddr[16];
    uint8_t   sname[64];
    uint8_t   file[128];
    uint32_t  magic;
    uint8_t   options[312];
} dhcp_pkt_t;

/* IP + UDP + DHCP 组合包 */
typedef struct {
    eth_hdr_t eth;
    ip_hdr_t  ip;
    udp_hdr_t udp;
    dhcp_pkt_t dhcp;
} ip_udp_dhcp_pkt_t;

#pragma pack(pop)

/* ========================================================================
 * 内部变量
 *
 * dhcp_state / my_ip 等标记为 volatile，防止编译器优化掉
 * 主循环中对这些变量的读取（可能被中断上下文间接影响）。
 * ======================================================================== */
static uint8_t my_mac[6];
static volatile uint32_t my_ip = 0;
static volatile uint32_t my_mask = 0;
static volatile uint32_t my_gateway = 0;
static volatile uint32_t my_dns = 0;
static volatile uint32_t dhcp_server = 0;
static uint32_t dhcp_lease = 0;
static uint32_t xid = 0x12345678;
static volatile dhcp_state_t dhcp_state = DHCP_STATE_IDLE;
static uint32_t dhcp_retry_count = 0;
static uint32_t dhcp_tick = 0;

/* 简易 ARP 缓存（单条目，记录最近通信对端的 MAC） */
static uint8_t arp_cache_mac[6] = {0};
static uint32_t arp_cache_ip = 0;

/* 接收缓冲区 */
static uint8_t rx_buf[1520];

static uint32_t ip_read_be(const void *data)
{
    const uint8_t *p = (const uint8_t *)data;

    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

static void ip_write_be(void *data, uint32_t ip)
{
    uint8_t *p = (uint8_t *)data;

    p[0] = (uint8_t)(ip >> 24);
    p[1] = (uint8_t)(ip >> 16);
    p[2] = (uint8_t)(ip >> 8);
    p[3] = (uint8_t)ip;
}

/* ========================================================================
 * 校验和计算
 * ======================================================================== */
static uint16_t ip_checksum(const uint16_t *buf, int len)
{
    uint32_t sum = 0;
    while (len > 1)
    {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1)
        sum += *(uint8_t *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (uint16_t)~sum;
}

/* ========================================================================
 * ARP 处理
 * ======================================================================== */
static void __attribute__((unused)) arp_send_request(uint32_t target_ip)
{
    arp_pkt_t pkt;

    memset(&pkt, 0, sizeof(pkt));
    memset(pkt.eth.dst, 0xFF, 6);           /* 广播 */
    memcpy(pkt.eth.src, my_mac, 6);
    pkt.eth.type = __builtin_bswap16(ETH_TYPE_ARP);

    pkt.htype = __builtin_bswap16(ARP_HTYPE_ETH);
    pkt.ptype = __builtin_bswap16(ARP_PTYPE_IP);
    pkt.hlen = 6;
    pkt.plen = 4;
    pkt.oper = __builtin_bswap16(ARP_OP_REQUEST);

    memcpy(pkt.sha, my_mac, 6);
    ip_write_be(pkt.spa, my_ip);
    memset(pkt.tha, 0, 6);
    ip_write_be(pkt.tpa, target_ip);

    eth_demo_send((uint8_t *)&pkt, sizeof(pkt));
}

/**
 * @brief 处理收到的 ARP 包（应答 ARP 请求 + 学习 ARP 缓存）
 */
static void arp_process(const uint8_t *frame, uint16_t len)
{
    const arp_pkt_t *arp = (const arp_pkt_t *)frame;
    uint16_t oper = __builtin_bswap16(arp->oper);

    /* 只处理 Ethernet + IPv4 类型 */
    if (__builtin_bswap16(arp->htype) != ARP_HTYPE_ETH) return;
    if (__builtin_bswap16(arp->ptype) != ARP_PTYPE_IP) return;
    if (arp->hlen != 6 || arp->plen != 4) return;

    /* 学习发送方的 MAC-IP 映射 */
    uint32_t sender_ip = ip_read_be(arp->spa);
    if (sender_ip != 0)
    {
        memcpy(arp_cache_mac, arp->sha, 6);
        arp_cache_ip = sender_ip;
    }

    if (oper == ARP_OP_REQUEST)
    {
        /* 检查是否请求的是我们的 IP */
        uint32_t target_ip = ip_read_be(arp->tpa);
        printf("[ARP] target=%08lX my=%08lX\r\n",
               (unsigned long)target_ip, (unsigned long)my_ip);
        if (target_ip != my_ip) return;

        /* 构造 ARP 应答 */
        arp_pkt_t reply;
        memset(&reply, 0, sizeof(reply));

        /* 以太网头：单播回请求方 */
        memcpy(reply.eth.dst, arp->sha, 6);
        memcpy(reply.eth.src, my_mac, 6);
        reply.eth.type = __builtin_bswap16(ETH_TYPE_ARP);

        /* ARP 头 */
        reply.htype = __builtin_bswap16(ARP_HTYPE_ETH);
        reply.ptype = __builtin_bswap16(ARP_PTYPE_IP);
        reply.hlen = 6;
        reply.plen = 4;
        reply.oper = __builtin_bswap16(ARP_OP_REPLY);

        /* 应答：我们的 MAC+IP → 请求方的 MAC+IP */
        memcpy(reply.sha, my_mac, 6);
        ip_write_be(reply.spa, my_ip);
        memcpy(reply.tha, arp->sha, 6);
        memcpy(reply.tpa, arp->spa, 4);

        {
            uint32_t ret = eth_demo_send((uint8_t *)&reply, sizeof(reply));
            printf("[ARP] TX=%lu DMASR=0x%08lX\r\n",
                   (unsigned long)ret, (unsigned long)ETH->DMASR);
        }
    }
}

/* ========================================================================
 * IP/UDP 发送
 * ======================================================================== */
void udp_send(uint32_t dst_ip, uint16_t dst_port,
                     uint16_t src_port, const uint8_t *data, uint16_t data_len)
{
    uint8_t buf[1520];
    ip_udp_dhcp_pkt_t *pkt = (ip_udp_dhcp_pkt_t *)buf;
    uint16_t udp_len = 8 + data_len;
    uint16_t ip_len = 20 + udp_len;

    memset(buf, 0, sizeof(buf));

    /* 以太网头 */
    if (dst_ip == 0xFFFFFFFF)
        memset(pkt->eth.dst, 0xFF, 6);
    else if (arp_cache_ip == dst_ip)
        memcpy(pkt->eth.dst, arp_cache_mac, 6);  /* ARP 缓存命中 */
    else
        memset(pkt->eth.dst, 0xFF, 6);  /* 缓存未命中，回退广播 */
    memcpy(pkt->eth.src, my_mac, 6);
    pkt->eth.type = __builtin_bswap16(ETH_TYPE_IP);

    /* IP 头 */
    pkt->ip.ver_ihl = IP_VER_IHL;
    pkt->ip.total_len = __builtin_bswap16(ip_len);
    pkt->ip.id = __builtin_bswap16(0x1234);
    pkt->ip.ttl = IP_TTL;
    pkt->ip.protocol = IP_PROTO_UDP;
    ip_write_be(&pkt->ip.src_ip, my_ip);
    ip_write_be(&pkt->ip.dst_ip, dst_ip);
    pkt->ip.checksum = ip_checksum((uint16_t *)&pkt->ip, 20);

    /* UDP 头 */
    pkt->udp.src_port = __builtin_bswap16(src_port);
    pkt->udp.dst_port = __builtin_bswap16(dst_port);
    pkt->udp.length = __builtin_bswap16(udp_len);

    /* 数据 */
    memcpy(buf + sizeof(eth_hdr_t) + sizeof(ip_hdr_t) + sizeof(udp_hdr_t),
           data, data_len);

    uint32_t total = sizeof(eth_hdr_t) + ip_len;
    eth_demo_send(buf, total);
}

/* ========================================================================
 * DHCP 发送
 * ======================================================================== */
static void dhcp_send_discover(void)
{
    uint8_t buf[512];
    dhcp_pkt_t *pkt = (dhcp_pkt_t *)buf;
    uint8_t *opt;
    int len;

    memset(buf, 0, sizeof(buf));

    pkt->op = DHCP_OP_BOOTREQUEST;
    pkt->htype = DHCP_HTYPE_ETH;
    pkt->hlen = DHCP_HLEN_ETH;
    pkt->xid = __builtin_bswap32(xid);
    pkt->flags = __builtin_bswap16(0x8000); /* 广播 */
    memcpy(pkt->chaddr, my_mac, 6);
    pkt->magic = __builtin_bswap32(DHCP_MAGIC);

    opt = pkt->options;
    *opt++ = OPT_MSG_TYPE;
    *opt++ = 1;
    *opt++ = DHCP_DISCOVER;

    /* 参数请求列表：子网掩码、路由器、DNS */
    *opt++ = 55; /* Parameter Request List */
    *opt++ = 3;
    *opt++ = OPT_SUBNET_MASK;
    *opt++ = OPT_ROUTER;
    *opt++ = OPT_DNS;

    /* 主机名 */
    *opt++ = 12; /* Host Name */
    *opt++ = 7;
    *opt++ = 'C'; *opt++ = 'H'; *opt++ = '3'; *opt++ = '2';
    *opt++ = 'V'; *opt++ = '3'; *opt++ = '0'; /* 不含结束符，长度=7 */

    *opt++ = OPT_END;

    len = (int)(opt - buf);
    udp_send(0xFFFFFFFF, UDP_PORT_BOOTPS, UDP_PORT_BOOTPC, buf, len);

#ifdef DHCP_DEBUG
    printf("[DHCP] DISCOVER sent\r\n");
#endif
}

static void dhcp_send_request(uint32_t offered_ip, uint32_t server_ip)
{
    uint8_t buf[512];
    dhcp_pkt_t *pkt = (dhcp_pkt_t *)buf;
    uint8_t *opt;
    int len;

    memset(buf, 0, sizeof(buf));

    pkt->op = DHCP_OP_BOOTREQUEST;
    pkt->htype = DHCP_HTYPE_ETH;
    pkt->hlen = DHCP_HLEN_ETH;
    pkt->xid = __builtin_bswap32(xid);
    pkt->flags = __builtin_bswap16(0x8000);
    memcpy(pkt->chaddr, my_mac, 6);
    pkt->magic = __builtin_bswap32(DHCP_MAGIC);

    opt = pkt->options;

    /* 消息类型 */
    *opt++ = OPT_MSG_TYPE;
    *opt++ = 1;
    *opt++ = DHCP_REQUEST;

    /* 请求的 IP */
    *opt++ = OPT_REQUESTED_IP;
    *opt++ = 4;
    ip_write_be(opt, offered_ip);
    opt += 4;

    /* DHCP 服务器 */
    *opt++ = OPT_SERVER_ID;
    *opt++ = 4;
    ip_write_be(opt, server_ip);
    opt += 4;

    *opt++ = OPT_END;

    len = (int)(opt - buf);
    udp_send(0xFFFFFFFF, UDP_PORT_BOOTPS, UDP_PORT_BOOTPC, buf, len);

#ifdef DHCP_DEBUG
    printf("[DHCP] REQUEST sent\r\n");
#endif
}

/* ========================================================================
 * DHCP 接收处理
 * ======================================================================== */
static void dhcp_process_reply(const uint8_t *data, uint16_t len)
{
    const dhcp_pkt_t *pkt = (const dhcp_pkt_t *)data;
    const uint8_t *opt;
    uint8_t msg_type = 0;
    uint32_t offered_ip, server_ip = 0;
    char ip_str[16];

    if (__builtin_bswap32(pkt->magic) != DHCP_MAGIC)
        return;

    if (__builtin_bswap32(pkt->xid) != xid)
        return;

    offered_ip = ip_read_be(&pkt->yiaddr);

    /* 解析选项 */
    opt = pkt->options;
    while (opt < data + len && *opt != OPT_END)
    {
        if (*opt == OPT_PAD)
        {
            opt++;
            continue;
        }
        uint8_t opt_len = opt[1];
        if (*opt == OPT_MSG_TYPE && opt_len == 1)
        {
            msg_type = opt[2];
        }
        opt += 2 + opt_len;
    }

    switch (msg_type)
    {
    case DHCP_OFFER:
        dhcp_ip_to_str(offered_ip, ip_str);
        printf("[DHCP] OFFER: IP=%s", ip_str);
        /* 找到服务器 ID */
        opt = pkt->options;
        while (opt < data + len && *opt != OPT_END)
        {
            if (*opt == OPT_SERVER_ID && opt[1] == 4)
            {
                server_ip = ip_read_be(opt + 2);
                break;
            }
            if (*opt == OPT_PAD) { opt++; continue; }
            opt += 2 + opt[1];
        }
        if (server_ip == 0)
        {
            server_ip = ip_read_be(&pkt->siaddr);
        }
        if (server_ip == 0)
        {
            printf(" without server id, ignored\r\n");
            break;
        }
        dhcp_ip_to_str(server_ip, ip_str);
        printf(" from %s\r\n", ip_str);

        dhcp_server = server_ip;
        dhcp_state = DHCP_STATE_REQUEST;
        dhcp_retry_count = 0;  /* 重置计数器 */
        dhcp_send_request(offered_ip, server_ip);
        break;

    case DHCP_ACK:
        my_ip = offered_ip;
        /* 解析子网掩码、网关、DNS */
        opt = pkt->options;
        while (opt < data + len && *opt != OPT_END)
        {
            if (*opt == OPT_PAD) { opt++; continue; }
            uint8_t olen = opt[1];
            if (*opt == OPT_SUBNET_MASK && olen == 4)
                my_mask = ip_read_be(opt + 2);
            else if (*opt == OPT_ROUTER && olen >= 4)
                my_gateway = ip_read_be(opt + 2);
            else if (*opt == OPT_DNS && olen >= 4)
                my_dns = ip_read_be(opt + 2);
            else if (*opt == OPT_LEASE_TIME && olen == 4)
                dhcp_lease = ip_read_be(opt + 2);
            opt += 2 + olen;
        }

        dhcp_ip_to_str(my_ip, ip_str);
        printf("[DHCP] ACK: IP=%s", ip_str);
        dhcp_ip_to_str(my_mask, ip_str);
        printf(" Mask=%s", ip_str);
        dhcp_ip_to_str(my_gateway, ip_str);
        printf(" GW=%s", ip_str);
        dhcp_ip_to_str(my_dns, ip_str);
        printf(" DNS=%s", ip_str);
        printf("\r\n");

        dhcp_state = DHCP_STATE_BOUND;
        break;

    case DHCP_NAK:
        printf("[DHCP] NAK received\r\n");
        dhcp_state = DHCP_STATE_IDLE;
        break;
    }
}

/* ========================================================================
 * IP/UDP 接收处理
 * ======================================================================== */
static void ip_process(const uint8_t *frame, uint16_t len)
{
    const ip_hdr_t *ip = (const ip_hdr_t *)(frame + 14);

    if (ip->protocol != IP_PROTO_UDP)
        return;

    const udp_hdr_t *udp = (const udp_hdr_t *)(frame + 14 + 20);
    uint16_t dst_port = __builtin_bswap16(udp->dst_port);
    uint16_t src_port = __builtin_bswap16(udp->src_port);
    uint16_t udp_len = __builtin_bswap16(udp->length);
    const uint8_t *payload = frame + 14 + 20 + 8;
    uint32_t src_ip = ip_read_be(&ip->src_ip);

    if (dst_port == UDP_PORT_BOOTPC)
    {
        /* DHCP 应答 */
        dhcp_process_reply(payload, udp_len - 8);
    }
    else if (dst_port == 7)
    {
        /* UDP Echo: 原样回显数据到发送方 */
        if (udp_len > 8)
        {
            udp_send(src_ip, src_port, 7, payload, udp_len - 8);
            printf("[ECHO] %d bytes echoed to port %d\r\n", udp_len - 8, src_port);
        }
    }
}

/* ========================================================================
 * 公共接口实现
 * ======================================================================== */

void dhcp_init(void)
{
    eth_demo_get_mac_addr(my_mac);
    xid = (uint32_t)(my_mac[2] << 24 | my_mac[3] << 16 | my_mac[4] << 8 | my_mac[5]);
    dhcp_state = DHCP_STATE_DISCOVER;
    dhcp_retry_count = 0;
    dhcp_tick = 0;
    my_ip = 0;

    printf("[DHCP] Client initialized, MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
           my_mac[0], my_mac[1], my_mac[2], my_mac[3], my_mac[4], my_mac[5]);
}

/**
 * @brief 设置静态 IP（跳过 DHCP，用于调试）
 */
void dhcp_set_static_ip(uint32_t ip, uint32_t mask, uint32_t gw, uint32_t dns)
{
    my_ip = ip;
    my_mask = mask;
    my_gateway = gw;
    my_dns = dns;
    dhcp_state = DHCP_STATE_BOUND;
    printf("[DHCP] Static IP set\r\n");
}

void dhcp_poll(void)
{
    uint16_t rx_len;

    /* 每次调用都检查接收 */
    rx_len = eth_demo_recv(rx_buf);
    if (rx_len > 0)
    {
        uint16_t eth_type = __builtin_bswap16(*(uint16_t *)(rx_buf + 12));
        if (eth_type == ETH_TYPE_IP)
        {
            ip_process(rx_buf, rx_len);
        }
        else if (eth_type == ETH_TYPE_ARP)
        {
            arp_process(rx_buf, rx_len);
        }
    }

    /* 状态机重试计时 */
    dhcp_tick++;
    if (dhcp_tick < 100) return; /* 约 1 秒间隔 */
    dhcp_tick = 0;

    switch (dhcp_state)
    {
    case DHCP_STATE_DISCOVER:
        if (eth_demo_get_link_status() != ETH_LINK_UP)
            break;
        dhcp_send_discover();
        dhcp_retry_count++;
        if (dhcp_retry_count > 10)
        {
            printf("[DHCP] Timeout, no DHCP server found\r\n");
            dhcp_state = DHCP_STATE_ERROR;
        }
        break;

    case DHCP_STATE_REQUEST:
        dhcp_retry_count++;
        if (dhcp_retry_count > 5)
        {
            printf("[DHCP] Request timeout, retrying DISCOVER\r\n");
            dhcp_state = DHCP_STATE_DISCOVER;
            dhcp_retry_count = 0;
        }
        break;

    case DHCP_STATE_BOUND:
        break;

    default:
        break;
    }
}

dhcp_state_t dhcp_get_state(void)
{
    return dhcp_state;
}

int dhcp_get_info(dhcp_info_t *info)
{
    if (dhcp_state != DHCP_STATE_BOUND)
        return -1;

    info->ip = my_ip;
    info->mask = my_mask;
    info->gateway = my_gateway;
    info->dns = my_dns;
    info->server = dhcp_server;
    info->lease = dhcp_lease;
    return 0;
}

void dhcp_ip_to_str(uint32_t ip, char *buf)
{
    snprintf(buf, 16, "%u.%u.%u.%u",
             (unsigned)((ip >> 24) & 0xFF), (unsigned)((ip >> 16) & 0xFF),
             (unsigned)((ip >> 8) & 0xFF), (unsigned)(ip & 0xFF));
}
