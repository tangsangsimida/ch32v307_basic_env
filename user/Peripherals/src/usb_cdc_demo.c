/**
 * @file usb_cdc_demo.c
 * @brief USB CDC (Virtual COM Port) Demo 实现
 *
 * 参考 WCH 官方 SimulateCDC demo，适配到当前项目架构。
 * 去掉 UART 桥接，USB 收到数据直接回显。
 * 使用 USBFS 控制器 (PA11/PA12)，Full-Speed 模式。
 */

#include "usb_cdc_demo.h"
#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "debug.h"
#include <string.h>

/* ========================================================================
 * USB 描述符配置
 * ======================================================================== */
#define DEF_USB_VID             0x1A86
#define DEF_USB_PID             0xFE0C
#define DEF_USB_EP0_SIZE        64
#define DEF_USB_EP1_SIZE        64  /* Interrupt IN - CDC 通知 */
#define DEF_USB_EP2_SIZE        64  /* Bulk OUT - 接收数据 */
#define DEF_USB_EP3_SIZE        64  /* Bulk IN  - 发送数据 */

/* 设备描述符 */
static const uint8_t DevDescr[] = {
    0x12,       /* bLength */
    0x01,       /* bDescriptorType */
    0x10, 0x01, /* bcdUSB 1.10 */
    0x02,       /* bDeviceClass: CDC */
    0x00,       /* bDeviceSubClass */
    0x00,       /* bDeviceProtocol */
    DEF_USB_EP0_SIZE,
    (uint8_t)DEF_USB_VID, (uint8_t)(DEF_USB_VID >> 8),
    (uint8_t)DEF_USB_PID, (uint8_t)(DEF_USB_PID >> 8),
    0x01, 0x00, /* bcdDevice */
    0x01,       /* iManufacturer */
    0x02,       /* iProduct */
    0x00,       /* iSerialNumber */
    0x01,       /* bNumConfigurations */
};

/* 配置描述符 */
static const uint8_t CfgDescr[] = {
    /* 配置描述符 */
    0x09, 0x02, 0x43, 0x00, 0x02, 0x01, 0x00, 0x80, 0x32,

    /* 接口 0 (CDC 通信) */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,

    /* CDC 功能描述符 */
    0x05, 0x24, 0x00, 0x10, 0x01,                         /* Header */
    0x05, 0x24, 0x01, 0x00, 0x01,                         /* ACM */
    0x04, 0x24, 0x02, 0x02,                               /* Union */
    0x05, 0x24, 0x06, 0x00, 0x01,                         /* CDC */

    /* 中断端点 (EP1 IN) */
    0x07, 0x05, 0x81, 0x03, DEF_USB_EP1_SIZE, 0x00, 0x01,

    /* 接口 1 (CDC 数据) */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,

    /* Bulk OUT 端点 (EP2) */
    0x07, 0x05, 0x02, 0x02, DEF_USB_EP2_SIZE, 0x00, 0x00,

    /* Bulk IN 端点 (EP3) */
    0x07, 0x05, 0x83, 0x02, DEF_USB_EP3_SIZE, 0x00, 0x00,
};

/* 字符串描述符 */
static const uint8_t LangDescr[]  = { 0x04, 0x03, 0x09, 0x04 };
static const uint8_t ManuDescr[]  = { 0x0E, 0x03, 'w',0, 'c',0, 'h',0, '.',0, 'c',0, 'n',0 };
static const uint8_t ProdDescr[]  = { 0x16, 0x03, 'U',0, 'S',0, 'B',0, ' ',0, 'S',0, 'e',0, 'r',0, 'i',0, 'a',0, 'l',0 };

/* CDC LINE_CODING 缓存 */
static uint8_t LineCoding[7] = { 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08 };

/* ========================================================================
 * USBFS 寄存器操作宏
 * ======================================================================== */
#define USBFSD_UEP_MOD_BASE     0x5000000C
#define USBFSD_UEP_DMA_BASE     0x50000010
#define USBFSD_UEP_LEN_BASE     0x50000030
#define USBFSD_UEP_CTL_BASE     0x50000032

#define USBFSD_UEP_MOD(n)       (*((volatile uint8_t *)(USBFSD_UEP_MOD_BASE + n)))
#define USBFSD_UEP_TX_CTRL(n)   (*((volatile uint8_t *)(USBFSD_UEP_CTL_BASE + n * 0x04)))
#define USBFSD_UEP_RX_CTRL(n)   (*((volatile uint8_t *)(USBFSD_UEP_CTL_BASE + n * 0x04 + 1)))
#define USBFSD_UEP_DMA(n)       (*((volatile uint32_t *)(USBFSD_UEP_DMA_BASE + n * 0x04)))
#define USBFSD_UEP_TLEN(n)      (*((volatile uint16_t *)(USBFSD_UEP_LEN_BASE + n * 0x04)))

/* ========================================================================
 * 端点缓冲区
 * ======================================================================== */
__attribute__((aligned(4))) static uint8_t EP0_Buf[DEF_USB_EP0_SIZE];
__attribute__((aligned(4))) static uint8_t EP1_Buf[DEF_USB_EP1_SIZE];
__attribute__((aligned(4))) static uint8_t EP2_Buf[DEF_USB_EP2_SIZE]; /* 接收 */
__attribute__((aligned(4))) static uint8_t EP3_Buf[DEF_USB_EP3_SIZE]; /* 发送 */

/* 接收环形缓冲区（大小必须是 2 的幂，用于位掩码取模） */
#define USB_RX_BUF_SIZE     512
#define USB_RX_BUF_MASK     (USB_RX_BUF_SIZE - 1)
static uint8_t usb_rx_buf[USB_RX_BUF_SIZE];
static volatile uint16_t usb_rx_head = 0;
static volatile uint16_t usb_rx_tail = 0;

/* 状态变量 */
static const uint8_t *pDescr;
static volatile uint8_t SetupReqCode;
static volatile uint8_t SetupReqType;
static volatile uint16_t SetupReqValue;
static volatile uint16_t SetupReqIndex;
static volatile uint16_t SetupReqLen;
static volatile uint8_t DevConfig = 0;
static volatile uint8_t DevAddr = 0;
static volatile uint8_t DevSleepStatus = 0;
static volatile uint8_t DevEnumStatus = 0;
static volatile uint8_t Endp_Busy[8];

/* Setup 请求包指针 */
#define pSetupReq ((PUSB_SETUP_REQ)EP0_Buf)

/* 端点方向 */
#define DEF_UEP_IN  0x80

/* ========================================================================
 * USBFS 时钟和设备初始化
 * ======================================================================== */

static void usbfs_rcc_init(void)
{
#ifdef CH32V30x_D8C
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
    RCC_USBHSConfig(RCC_USBPLL_Div2);
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
#else
    if (SystemCoreClock == 144000000)
        RCC_USBFSCLKConfig(RCC_USBFSCLKSource_PLLCLK_Div3);
    else if (SystemCoreClock == 96000000)
        RCC_USBFSCLKConfig(RCC_USBFSCLKSource_PLLCLK_Div2);
    else if (SystemCoreClock == 48000000)
        RCC_USBFSCLKConfig(RCC_USBFSCLKSource_PLLCLK_Div1);
#endif
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBFS, ENABLE);
}

static void usbfs_endp_init(void)
{
    USBFSD->UEP4_1_MOD = USBFS_UEP1_TX_EN;
    USBFSD->UEP2_3_MOD = USBFS_UEP2_RX_EN | USBFS_UEP3_TX_EN;

    USBFSD->UEP0_DMA = (uint32_t)EP0_Buf;
    USBFSD->UEP1_DMA = (uint32_t)EP1_Buf;
    USBFSD->UEP2_DMA = (uint32_t)EP2_Buf;
    USBFSD->UEP3_DMA = (uint32_t)EP3_Buf;

    USBFSD->UEP0_RX_CTRL = USBFS_UEP_R_RES_ACK;
    USBFSD->UEP2_RX_CTRL = USBFS_UEP_R_RES_ACK;

    USBFSD->UEP1_TX_LEN = 0;
    USBFSD->UEP3_TX_LEN = 0;

    USBFSD->UEP0_TX_CTRL = USBFS_UEP_T_RES_NAK;
    USBFSD->UEP1_TX_CTRL = USBFS_UEP_T_RES_NAK;
    USBFSD->UEP3_TX_CTRL = USBFS_UEP_T_RES_NAK;

    for (uint8_t i = 0; i < 8; i++)
    {
        Endp_Busy[i] = 0;
    }
}

static void usbfs_device_init(void)
{
    USBFSH->BASE_CTRL = USBFS_UC_RESET_SIE | USBFS_UC_CLR_ALL;
    Delay_Us(10);
    USBFSH->BASE_CTRL = 0x00;
    USBFSD->INT_EN = USBFS_UIE_SUSPEND | USBFS_UIE_BUS_RST | USBFS_UIE_TRANSFER;
    USBFSD->BASE_CTRL = USBFS_UC_DEV_PU_EN | USBFS_UC_INT_BUSY | USBFS_UC_DMA_EN;
    usbfs_endp_init();
    USBFSD->UDEV_CTRL = USBFS_UD_PD_DIS | USBFS_UD_PORT_EN;
    NVIC_EnableIRQ(USBFS_IRQn);
}

/* ========================================================================
 * USB 数据上传辅助
 * ======================================================================== */
static uint8_t usb_endp_data_up(uint8_t endp, const uint8_t *pbuf, uint16_t len)
{
    if (endp < 1 || endp > 7) return 1;
    if (Endp_Busy[endp]) return 1;

    memcpy((uint8_t *)(USBFSD_UEP_DMA(endp) + 0x20000000), pbuf, len);
    Endp_Busy[endp] = 1;
    USBFSD_UEP_TLEN(endp) = len;
    USBFSD_UEP_TX_CTRL(endp) = (USBFSD_UEP_TX_CTRL(endp) & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_ACK;

    return 0;
}

/* ========================================================================
 * USB 中断处理
 * ======================================================================== */
void USBFS_IRQHandler(void) __attribute__((interrupt("machine")));
void USBFS_IRQHandler(void)
{
    uint8_t intflag, intst, errflag;
    uint16_t len;

    intflag = USBFSD->INT_FG;
    intst = USBFSD->INT_ST;

    if (intflag & USBFS_UIF_TRANSFER)
    {
        switch (intst & USBFS_UIS_TOKEN_MASK)
        {
        /* ---- TOKEN IN ---- */
        case USBFS_UIS_TOKEN_IN:
            switch (intst & (USBFS_UIS_TOKEN_MASK | USBFS_UIS_ENDP_MASK))
            {
            case (USBFS_UIS_TOKEN_IN | 0): /* EP0 IN */
                if (SetupReqLen == 0)
                {
                    USBFSD->UEP0_RX_CTRL = USBFS_UEP_R_TOG | USBFS_UEP_R_RES_ACK;
                }
                if ((SetupReqType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD)
                {
                    switch (SetupReqCode)
                    {
                    case USB_GET_DESCRIPTOR:
                        len = SetupReqLen >= DEF_USB_EP0_SIZE ? DEF_USB_EP0_SIZE : SetupReqLen;
                        memcpy(EP0_Buf, pDescr, len);
                        SetupReqLen -= len;
                        pDescr += len;
                        USBFSD->UEP0_TX_LEN = len;
                        USBFSD->UEP0_TX_CTRL ^= USBFS_UEP_T_TOG;
                        break;
                    case USB_SET_ADDRESS:
                        USBFSD->DEV_ADDR = (USBFSD->DEV_ADDR & USBFS_UDA_GP_BIT) | DevAddr;
                        break;
                    default:
                        break;
                    }
                }
                break;

            case (USBFS_UIS_TOKEN_IN | 1): /* EP1 IN */
                USBFSD->UEP1_TX_CTRL ^= USBFS_UEP_T_TOG;
                USBFSD->UEP1_TX_CTRL = (USBFSD->UEP1_TX_CTRL & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_NAK;
                Endp_Busy[1] = 0;
                break;

            case (USBFS_UIS_TOKEN_IN | 3): /* EP3 IN */
                USBFSD->UEP3_TX_CTRL ^= USBFS_UEP_T_TOG;
                USBFSD->UEP3_TX_CTRL = (USBFSD->UEP3_TX_CTRL & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_NAK;
                Endp_Busy[3] = 0;
                break;

            default:
                break;
            }
            break;

        /* ---- TOKEN OUT ---- */
        case USBFS_UIS_TOKEN_OUT:
            switch (intst & (USBFS_UIS_TOKEN_MASK | USBFS_UIS_ENDP_MASK))
            {
            case (USBFS_UIS_TOKEN_OUT | 0): /* EP0 OUT */
                len = USBFSD->RX_LEN;
                if (intst & USBFS_UIS_TOG_OK)
                {
                    if ((SetupReqType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
                    {
                        SetupReqLen = 0;
                        if (SetupReqCode == CDC_SET_LINE_CODING && len >= 7)
                        {
                            memcpy(LineCoding, EP0_Buf, 7);
                        }
                    }
                    if (SetupReqLen == 0)
                    {
                        USBFSD->UEP0_TX_LEN = 0;
                        USBFSD->UEP0_TX_CTRL = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
                    }
                }
                break;

            case (USBFS_UIS_TOKEN_OUT | 2): /* EP2 OUT - 接收数据 */
                USBFSD->UEP2_RX_CTRL ^= USBFS_UEP_R_TOG;
                len = USBFSD->RX_LEN;
                if (len > 0)
                {
                    /* 将数据存入环形缓冲区 */
                    for (uint16_t i = 0; i < len; i++)
                    {
                        uint16_t next = (usb_rx_head + 1) & USB_RX_BUF_MASK;
                        if (next != usb_rx_tail)
                        {
                            usb_rx_buf[usb_rx_head] = EP2_Buf[i];
                            usb_rx_head = next;
                        }
                    }
                }
                break;

            default:
                break;
            }
            break;

        /* ---- SETUP ---- */
        case USBFS_UIS_TOKEN_SETUP:
            USBFSD->UEP0_TX_CTRL = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_NAK;
            USBFSD->UEP0_RX_CTRL = USBFS_UEP_R_TOG | USBFS_UEP_R_RES_NAK;

            SetupReqType = pSetupReq->bRequestType;
            SetupReqCode = pSetupReq->bRequest;
            SetupReqLen = pSetupReq->wLength;
            SetupReqValue = pSetupReq->wValue;
            SetupReqIndex = pSetupReq->wIndex;
            len = 0;
            errflag = 0;

            if ((SetupReqType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
            {
                /* 非标准请求 */
                if (SetupReqType & USB_REQ_TYP_CLASS)
                {
                    switch (SetupReqCode)
                    {
                    case CDC_GET_LINE_CODING:
                        pDescr = LineCoding;
                        len = 7;
                        break;
                    case CDC_SET_LINE_CODING:
                        break;
                    case CDC_SET_LINE_CTLSTE:
                        break;
                    case CDC_SEND_BREAK:
                        break;
                    default:
                        errflag = 0xFF;
                        break;
                    }
                }
                else
                {
                    errflag = 0xFF;
                }

                len = (SetupReqLen >= DEF_USB_EP0_SIZE) ? DEF_USB_EP0_SIZE : SetupReqLen;
                memcpy(EP0_Buf, pDescr, len);
                pDescr += len;
            }
            else
            {
                /* 标准请求 */
                switch (SetupReqCode)
                {
                case USB_GET_DESCRIPTOR:
                    switch ((uint8_t)(SetupReqValue >> 8))
                    {
                    case USB_DESCR_TYP_DEVICE:
                        pDescr = DevDescr;
                        len = sizeof(DevDescr);
                        break;
                    case USB_DESCR_TYP_CONFIG:
                        pDescr = CfgDescr;
                        len = sizeof(CfgDescr);
                        break;
                    case USB_DESCR_TYP_STRING:
                        switch ((uint8_t)(SetupReqValue & 0xFF))
                        {
                        case 0: pDescr = LangDescr; len = sizeof(LangDescr); break;
                        case 1: pDescr = ManuDescr; len = sizeof(ManuDescr); break;
                        case 2: pDescr = ProdDescr; len = sizeof(ProdDescr); break;
                        default: errflag = 0xFF; break;
                        }
                        break;
                    default:
                        errflag = 0xFF;
                        break;
                    }
                    if (SetupReqLen > len) SetupReqLen = len;
                    len = (SetupReqLen >= DEF_USB_EP0_SIZE) ? DEF_USB_EP0_SIZE : SetupReqLen;
                    memcpy(EP0_Buf, pDescr, len);
                    pDescr += len;
                    break;

                case USB_SET_ADDRESS:
                    DevAddr = (uint8_t)(SetupReqValue & 0xFF);
                    break;

                case USB_GET_CONFIGURATION:
                    EP0_Buf[0] = DevConfig;
                    if (SetupReqLen > 1) SetupReqLen = 1;
                    break;

                case USB_SET_CONFIGURATION:
                    DevConfig = (uint8_t)(SetupReqValue & 0xFF);
                    DevEnumStatus = 0x01;
                    break;

                case USB_CLEAR_FEATURE:
                    if ((SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)
                    {
                        if ((uint8_t)(SetupReqValue & 0xFF) == USB_REQ_FEAT_REMOTE_WAKEUP)
                        {
                            DevSleepStatus &= ~0x01;
                        }
                    }
                    else if ((SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                    {
                        if ((uint8_t)(SetupReqValue & 0xFF) == USB_REQ_FEAT_ENDP_HALT)
                        {
                            switch ((uint8_t)(SetupReqIndex & 0xFF))
                            {
                            case (0x80 | 1): USBFSD->UEP1_TX_CTRL = USBFS_UEP_T_RES_NAK; break;
                            case (0x00 | 2): USBFSD->UEP2_RX_CTRL = USBFS_UEP_R_RES_ACK; break;
                            case (0x80 | 3): USBFSD->UEP3_TX_CTRL = USBFS_UEP_T_RES_NAK; break;
                            default: errflag = 0xFF; break;
                            }
                        }
                        else { errflag = 0xFF; }
                    }
                    else { errflag = 0xFF; }
                    break;

                case USB_SET_FEATURE:
                    if ((SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)
                    {
                        if ((uint8_t)(SetupReqValue & 0xFF) == USB_REQ_FEAT_REMOTE_WAKEUP)
                        {
                            if (CfgDescr[7] & 0x20) DevSleepStatus |= 0x01;
                            else errflag = 0xFF;
                        }
                        else { errflag = 0xFF; }
                    }
                    else if ((SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                    {
                        if ((uint8_t)(SetupReqValue & 0xFF) == USB_REQ_FEAT_ENDP_HALT)
                        {
                            switch ((uint8_t)(SetupReqIndex & 0xFF))
                            {
                            case (0x80 | 1):
                                USBFSD->UEP1_TX_CTRL = (USBFSD->UEP1_TX_CTRL & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_STALL;
                                break;
                            case (0x00 | 2):
                                USBFSD->UEP2_RX_CTRL = (USBFSD->UEP2_RX_CTRL & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_STALL;
                                break;
                            case (0x80 | 3):
                                USBFSD->UEP3_TX_CTRL = (USBFSD->UEP3_TX_CTRL & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_STALL;
                                break;
                            default: errflag = 0xFF; break;
                            }
                        }
                        else { errflag = 0xFF; }
                    }
                    else { errflag = 0xFF; }
                    break;

                case USB_GET_INTERFACE:
                    EP0_Buf[0] = 0x00;
                    if (SetupReqLen > 1) SetupReqLen = 1;
                    break;

                case USB_SET_INTERFACE:
                    break;

                case USB_GET_STATUS:
                    EP0_Buf[0] = 0x00;
                    EP0_Buf[1] = 0x00;
                    if (SetupReqLen > 2) SetupReqLen = 2;
                    break;

                default:
                    errflag = 0xFF;
                    break;
                }
            }

            if (errflag == 0xFF)
            {
                USBFSD->UEP0_TX_CTRL = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_STALL;
                USBFSD->UEP0_RX_CTRL = USBFS_UEP_R_TOG | USBFS_UEP_R_RES_STALL;
            }
            else
            {
                if (SetupReqType & DEF_UEP_IN)
                {
                    len = (SetupReqLen > DEF_USB_EP0_SIZE) ? DEF_USB_EP0_SIZE : SetupReqLen;
                    SetupReqLen -= len;
                    USBFSD->UEP0_TX_LEN = len;
                    USBFSD->UEP0_TX_CTRL = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
                }
                else
                {
                    if (SetupReqLen == 0)
                    {
                        USBFSD->UEP0_TX_LEN = 0;
                        USBFSD->UEP0_TX_CTRL = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
                    }
                    else
                    {
                        USBFSD->UEP0_RX_CTRL = USBFS_UEP_R_TOG | USBFS_UEP_R_RES_ACK;
                    }
                }
            }
            break;

        case USBFS_UIS_TOKEN_SOF:
            break;

        default:
            break;
        }
        USBFSD->INT_FG = USBFS_UIF_TRANSFER;
    }
    else if (intflag & USBFS_UIF_BUS_RST)
    {
        DevConfig = 0;
        DevAddr = 0;
        DevSleepStatus = 0;
        DevEnumStatus = 0;
        USBFSD->DEV_ADDR = 0;
        usbfs_endp_init();
        usb_rx_head = 0;
        usb_rx_tail = 0;
        USBFSD->INT_FG = USBFS_UIF_BUS_RST;
    }
    else if (intflag & USBFS_UIF_SUSPEND)
    {
        USBFSD->INT_FG = USBFS_UIF_SUSPEND;
        Delay_Us(10);
        if (USBFSD->MIS_ST & USBFS_UMS_SUSPEND)
        {
            DevSleepStatus |= 0x02;
        }
        else
        {
            DevSleepStatus &= ~0x02;
        }
    }
    else
    {
        USBFSD->INT_FG = intflag;
    }
}

/* ========================================================================
 * 公共接口实现
 * ======================================================================== */

void usb_cdc_demo_init(void)
{
    printf("[USB] Initializing USBFS CDC...\r\n");

    usbfs_rcc_init();
    usbfs_device_init();

    printf("[USB] CDC device ready, waiting for host enumeration\r\n");
}

uint8_t usb_cdc_is_enumerated(void)
{
    return DevEnumStatus;
}

uint8_t usb_cdc_send(const uint8_t *data, uint16_t len)
{
    if (!DevEnumStatus) return 1;
    if (len > DEF_USB_EP3_SIZE) len = DEF_USB_EP3_SIZE;
    return usb_endp_data_up(3, data, len);
}

uint16_t usb_cdc_available(void)
{
    if (usb_rx_head >= usb_rx_tail)
        return usb_rx_head - usb_rx_tail;
    else
        return USB_RX_BUF_SIZE - usb_rx_tail + usb_rx_head;
}

uint16_t usb_cdc_read(uint8_t *buf, uint16_t max_len)
{
    uint16_t count = 0;
    while (count < max_len && usb_rx_tail != usb_rx_head)
    {
        buf[count++] = usb_rx_buf[usb_rx_tail];
        usb_rx_tail = (usb_rx_tail + 1) & USB_RX_BUF_MASK;
    }
    return count;
}
