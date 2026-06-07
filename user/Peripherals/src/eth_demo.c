/**
 * @file eth_demo.c
 * @brief 以太网 MAC_RAW Demo 实现
 *
 * 使用 WCH 官方 SDK (ch32v30x_eth.c) 作为驱动层。
 * 仅实现应用层：RMII 引脚配置、PHY 初始化、链路检测、帧收发。
 */

#include "eth_demo.h"
#include "ch32v30x.h"
#include "ch32v30x_eth.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

/* ========================================================================
 * 配置
 * ======================================================================== */
#define ETH_TXBUFNB         2
#define ETH_RXBUFNB         2
#define ETH_BUF_SIZE        ETH_MAX_PACKET_SIZE  /* 1524，和 SDK 一致 */

/* ========================================================================
 * DMA 描述符和缓冲区（4 字节对齐）
 * ======================================================================== */
__attribute__((aligned(4))) static ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB];
__attribute__((aligned(4))) static ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB];
__attribute__((aligned(4))) static uint8_t MACTxBuf[ETH_TXBUFNB * ETH_BUF_SIZE];
__attribute__((aligned(4))) static uint8_t MACRxBuf[ETH_RXBUFNB * ETH_BUF_SIZE];

/* ========================================================================
 * 内部变量
 * ======================================================================== */
static uint8_t MACAddr[6];
static volatile uint8_t LinkSta = 0;  /* 0: down, 1: up */
static uint16_t phy_addr = 1;         /* PHY 地址（CH182 内部 PHY 地址为 1） */
static volatile uint32_t eth_rx_count = 0;
static volatile uint32_t eth_tx_count = 0;
static volatile uint32_t eth_err_count = 0;

/* ========================================================================
 * 内部 10M PHY 时钟配置（PLL3 = 60MHz）
 * ======================================================================== */
static void eth_10m_clock_init(void)
{
    RCC_PLL3Cmd(DISABLE);
    RCC_PREDIV2Config(RCC_PREDIV2_Div2);       /* HSE 8MHz / 2 = 4MHz */
    RCC_PLL3Config(RCC_PLL3Mul_15);            /* 4MHz × 15 = 60MHz */
    RCC_PLL3Cmd(ENABLE);
    while (RESET == RCC_GetFlagStatus(RCC_FLAG_PLL3RDY));
}

/* ========================================================================
 * 内部 10M PHY 使能
 * ======================================================================== */
static void eth_10m_phy_enable(void)
{
    /* 使能内部 10BASE-T PHY */
    EXTEN->EXTEN_CTR |= EXTEN_ETH_10M_EN;
}

/* ========================================================================
 * ETH 配置（使用官方 SDK）
 * ======================================================================== */
static void eth_configuration(uint8_t *mac_addr)
{
    ETH_InitTypeDef ETH_InitStructure;
    uint16_t timeout = 10000;

    /* 使能 ETH MAC 时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC |
                          RCC_AHBPeriph_ETH_MAC_Tx |
                          RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

    /* 配置 PLL3 60MHz 时钟（内部 10M PHY 需要） */
    eth_10m_clock_init();

    /* 使能内部 10BASE-T PHY */
    eth_10m_phy_enable();

    /* ETH 复位 */
    ETH_DeInit();

    /* ETH_DeInit 可能会关闭 MAC 时钟，重新使能 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC |
                          RCC_AHBPeriph_ETH_MAC_Tx |
                          RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

    /* 软件复位 */
    ETH_SoftwareReset();
    do {
        Delay_Us(10);
        if (!--timeout) break;
    } while (ETH->DMABMR & ETH_DMABMR_SR);

    /* MAC 配置 */
    ETH_InitStructure.ETH_Watchdog = ETH_Watchdog_Enable;
    ETH_InitStructure.ETH_Jabber = ETH_Jabber_Enable;
    ETH_InitStructure.ETH_InterFrameGap = ETH_InterFrameGap_96Bit;
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;

    /* 过滤配置 */
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
    ETH_InitStructure.ETH_PassControlFrames = ETH_PassControlFrames_BlockAll;
    ETH_InitStructure.ETH_DestinationAddrFilter = ETH_DestinationAddrFilter_Normal;
    ETH_InitStructure.ETH_SourceAddrFilter = ETH_SourceAddrFilter_Disable;
    ETH_InitStructure.ETH_HashTableHigh = 0x0;
    ETH_InitStructure.ETH_HashTableLow = 0x0;

    /* VLAN */
    ETH_InitStructure.ETH_VLANTagComparison = ETH_VLANTagComparison_16Bit;
    ETH_InitStructure.ETH_VLANTagIdentifier = 0x0;

    /* 流控 */
    ETH_InitStructure.ETH_PauseTime = 0x0;
    ETH_InitStructure.ETH_UnicastPauseFrameDetect = ETH_UnicastPauseFrameDetect_Disable;
    ETH_InitStructure.ETH_ReceiveFlowControl = ETH_ReceiveFlowControl_Disable;
    ETH_InitStructure.ETH_TransmitFlowControl = ETH_TransmitFlowControl_Disable;

    /* DMA */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Enable;
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Enable;
    ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
    ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
    ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
    ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_DescriptorSkipLength = 0x0;
    ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

    /* 写入 MAC/DMA 寄存器（和官方 demo 的 ETH_RegInit 一致） */
    ETH->MACMIIAR = (uint32_t)ETH_MACMIIAR_CR_Div42;
    ETH->MACCR = (uint32_t)(ETH_InitStructure.ETH_Watchdog |
                             ETH_InitStructure.ETH_Jabber |
                             ETH_InitStructure.ETH_InterFrameGap |
                             ETH_InitStructure.ETH_ChecksumOffload |
                             ETH_InitStructure.ETH_AutomaticPadCRCStrip |
                             ETH_InitStructure.ETH_LoopbackMode |
                             (1 << 9));  /* RD: Retry Disable */
    ETH->MACFFR = (uint32_t)(ETH_MACFFR_RA |  /* 接收所有帧（调试用） */
                              ETH_InitStructure.ETH_SourceAddrFilter |
                              ETH_InitStructure.ETH_PassControlFrames |
                              ETH_InitStructure.ETH_BroadcastFramesReception |
                              ETH_InitStructure.ETH_DestinationAddrFilter |
                              ETH_PromiscuousMode_Enable |
                              ETH_InitStructure.ETH_MulticastFramesFilter |
                              ETH_InitStructure.ETH_UnicastFramesFilter);
    ETH->MACHTHR = (uint32_t)ETH_InitStructure.ETH_HashTableHigh;
    ETH->MACHTLR = (uint32_t)ETH_InitStructure.ETH_HashTableLow;
    ETH->MACFCR = (uint32_t)((ETH_InitStructure.ETH_PauseTime << 16) |
                              ETH_InitStructure.ETH_UnicastPauseFrameDetect |
                              ETH_InitStructure.ETH_ReceiveFlowControl |
                              ETH_InitStructure.ETH_TransmitFlowControl);
    ETH->MACVLANTR = (uint32_t)(ETH_InitStructure.ETH_VLANTagComparison |
                                 ETH_InitStructure.ETH_VLANTagIdentifier);
    ETH->DMAOMR = (uint32_t)(ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame |
                              ETH_InitStructure.ETH_TransmitStoreForward |
                              ETH_InitStructure.ETH_ForwardErrorFrames |
                              ETH_InitStructure.ETH_ForwardUndersizedGoodFrames);

    /* 复位内部 10M PHY */
    ETH_WritePHYRegister(phy_addr, PHY_BCR, PHY_Reset);
    Delay_Ms(100);

    /* 配置 MAC 地址 */
    ETH->MACA0HR = (uint32_t)((mac_addr[5] << 8) | mac_addr[4]);
    ETH->MACA0LR = (uint32_t)(mac_addr[0] | (mac_addr[1] << 8) |
                               (mac_addr[2] << 16) | (mac_addr[3] << 24));

    /* 屏蔽 MMC 中断 */
    ETH->MMCTIMR = ETH_MMCTIMR_TGFM;
    ETH->MMCRIMR = ETH_MMCRIMR_RGUFM | ETH_MMCRIMR_RFCEM;

    /* 使能 DMA 中断 */
    ETH_DMAITConfig(ETH_DMA_IT_NIS |
                    ETH_DMA_IT_R |
                    ETH_DMA_IT_T |
                    ETH_DMA_IT_AIS |
                    ETH_DMA_IT_RBU, ENABLE);
}

/* ========================================================================
 * 链路状态处理
 * ======================================================================== */
static void eth_check_link(void)
{
    uint16_t bsr;
    static uint8_t link_down_count = 0;

    (void)ETH_ReadPHYRegister(phy_addr, PHY_BSR);
    bsr = ETH_ReadPHYRegister(phy_addr, PHY_BSR);

    if (bsr & PHY_Linked_Status)
    {
        link_down_count = 0;
        if (LinkSta == 0)
        {
            /* 内部 10M PHY：10Mbps，根据 BCR 判断双工 */
            uint16_t bcr = ETH_ReadPHYRegister(phy_addr, PHY_BCR);
            ETH->MACCR &= ~(ETH_Speed_100M | ETH_Speed_1000M);
            if (bcr & (1 << 8))
                ETH->MACCR |= ETH_Mode_FullDuplex;
            else
                ETH->MACCR &= ~ETH_Mode_FullDuplex;

            LinkSta = 1;
            printf("[ETH] Link UP (10M %s-duplex)\r\n",
                   (bcr & (1 << 8)) ? "full" : "half");
        }
    }
    else
    {
        if (link_down_count < 3)
        {
            link_down_count++;
            return;
        }

        if (LinkSta == 1)
        {
            LinkSta = 0;
            printf("[ETH] Link DOWN\r\n");
        }
    }
}

/* ========================================================================
 * MAC 地址读取（从芯片 ROM）
 * ======================================================================== */
static void eth_get_mac_from_rom(uint8_t *mac)
{
    uint8_t *macaddr = (uint8_t *)(0x1FFFF7E8 + 5);
    for (int i = 0; i < 6; i++)
    {
        mac[i] = *macaddr;
        macaddr--;
    }
}

/* ========================================================================
 * 公共接口实现
 * ======================================================================== */

void eth_demo_init(void)
{
    /* 获取 MAC 地址 */
    eth_get_mac_from_rom(MACAddr);
    printf("[ETH] MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           MACAddr[0], MACAddr[1], MACAddr[2],
           MACAddr[3], MACAddr[4], MACAddr[5]);

    /* ETH 初始化（内部 10M PHY） */
    eth_configuration(MACAddr);

    /* 初始化 DMA 描述符链 */
    ETH_DMATxDescChainInit(DMATxDscrTab, MACTxBuf, ETH_TXBUFNB);
    ETH_DMARxDescChainInit(DMARxDscrTab, MACRxBuf, ETH_RXBUFNB);

    /* 启动 ETH MAC */
    ETH_Start();

    /* 使能 ETH 中断 */
    NVIC_EnableIRQ(ETH_IRQn);
    NVIC_SetPriority(ETH_IRQn, 0);

    /* 启动自协商 */
    ETH_WritePHYRegister(phy_addr, PHY_BCR, PHY_AutoNegotiation | PHY_Restart_AutoNegotiation);
    printf("[ETH] 10M PHY initialized, auto-negotiation started\r\n");
}

void eth_demo_poll(void)
{
    static uint32_t poll_count = 0;
    static uint32_t diag_count = 0;

    if (++poll_count >= 100)
    {
        poll_count = 0;
        eth_check_link();
    }

    /* 每 5 秒输出 DMA 状态 */
    if (LinkSta == 1 && ++diag_count >= 500)
    {
        diag_count = 0;
        printf("[ETH] DMASR=0x%08lX RDesc.St=0x%08lX RX=%lu TX=%lu ERR=%lu\r\n",
               ETH->DMASR, ((volatile uint32_t *)ETH->DMARDLAR)[0],
               eth_rx_count, eth_tx_count, eth_err_count);
    }
}

uint32_t eth_demo_send(uint8_t *pbuf, uint16_t len)
{
    if (len > ETH_MAX_PACKET_SIZE) return 0;
    return ETH_HandleTxPkt(pbuf, len);
}

uint32_t eth_demo_recv(uint8_t *pbuf)
{
    uint32_t len = 0;

    len = ETH_GetRxPktSize();
    if (len > 0)
    {
        ETH_HandleRxPkt(pbuf);
    }

    return len;
}

eth_link_status_t eth_demo_get_link_status(void)
{
    return (eth_link_status_t)LinkSta;
}

void eth_demo_get_mac_addr(uint8_t *mac)
{
    memcpy(mac, MACAddr, 6);
}

/* ========================================================================
 * ETH 中断处理
 * ======================================================================== */
void ETH_IRQHandler(void) __attribute__((interrupt("machine")));
void ETH_IRQHandler(void)
{
    uint32_t int_sta;

    int_sta = ETH->DMASR;

    if (int_sta & ETH_DMA_IT_AIS)
    {
        if (int_sta & ETH_DMA_IT_RBU)
        {
            eth_err_count++;
            ETH_DMAClearITPendingBit(ETH_DMA_IT_RBU);
        }
        ETH_DMAClearITPendingBit(ETH_DMA_IT_AIS);
    }

    if (int_sta & ETH_DMA_IT_NIS)
    {
        if (int_sta & ETH_DMA_IT_R)
        {
            eth_rx_count++;
            ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
        }
        if (int_sta & ETH_DMA_IT_T)
        {
            ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
        }
        ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
    }
}
