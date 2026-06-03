/**
 * @file hal_can.c
 * @brief CAN硬件抽象层CH32V30x实现
 */

#include "hal_can.h"
#include "ch32v30x.h"
#include <stddef.h>

static void (*can_callback)(hal_can_msg_t *msg) = NULL;

int hal_can_init(const hal_can_config_t *config)
{
    CAN_InitTypeDef can_init;
    CAN_FilterInitTypeDef filter_init;

    if (config == NULL || config->id >= HAL_CAN_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /* 配置CAN */
    can_init.CAN_TTCM = DISABLE;
    can_init.CAN_ABOM = DISABLE;
    can_init.CAN_AWUM = DISABLE;
    can_init.CAN_NART = DISABLE;
    can_init.CAN_RFLM = DISABLE;
    can_init.CAN_TXFP = DISABLE;

    switch (config->mode)
    {
        case 1:
            can_init.CAN_Mode = CAN_Mode_LoopBack;
            break;
        case 2:
            can_init.CAN_Mode = CAN_Mode_Silent;
            break;
        default:
            can_init.CAN_Mode = CAN_Mode_Normal;
            break;
    }

    /* 设置波特率 */
    can_init.CAN_SJW = CAN_SJW_1tq;
    can_init.CAN_BS1 = CAN_BS1_3tq;
    can_init.CAN_BS2 = CAN_BS2_2tq;
    can_init.CAN_Prescaler = SystemCoreClock / config->baudrate / 12;

    CAN_Init(CAN1, &can_init);

    /* 配置过滤器 */
    filter_init.CAN_FilterNumber = 0;
    filter_init.CAN_FilterMode = CAN_FilterMode_IdMask;
    filter_init.CAN_FilterScale = CAN_FilterScale_32bit;
    filter_init.CAN_FilterIdHigh = 0x0000;
    filter_init.CAN_FilterIdLow = 0x0000;
    filter_init.CAN_FilterMaskIdHigh = 0x0000;
    filter_init.CAN_FilterMaskIdLow = 0x0000;
    filter_init.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    filter_init.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&filter_init);

    return 0;
}

int hal_can_deinit(hal_can_id_t id)
{
    if (id >= HAL_CAN_MAX)
    {
        return -1;
    }

    CAN_DeInit(CAN1);
    return 0;
}

int hal_can_transmit(hal_can_id_t id, const hal_can_msg_t *msg, uint32_t timeout_ms)
{
    CanTxMsg tx_msg;

    if (id >= HAL_CAN_MAX || msg == NULL)
    {
        return -1;
    }

    /* 配置发送消息 */
    if (msg->ide)
    {
        tx_msg.ExtId = msg->id;
        tx_msg.IDE = CAN_Id_Extended;
    }
    else
    {
        tx_msg.StdId = msg->id;
        tx_msg.IDE = CAN_Id_Standard;
    }

    tx_msg.RTR = msg->rtr ? CAN_RTR_Remote : CAN_RTR_Data;
    tx_msg.DLC = msg->dlc;

    for (uint8_t i = 0; i < msg->dlc; i++)
    {
        tx_msg.Data[i] = msg->data[i];
    }

    /* 发送 */
    uint8_t mailbox = CAN_Transmit(CAN1, &tx_msg);

    /* 等待发送完成 */
    uint32_t timeout = timeout_ms;
    while (CAN_TransmitStatus(CAN1, mailbox) != CAN_TxStatus_Ok)
    {
        if (timeout == 0)
        {
            return -1;
        }
        hal_delay_ms(1);
        timeout--;
    }

    return 0;
}

int hal_can_receive(hal_can_id_t id, hal_can_msg_t *msg, uint32_t timeout_ms)
{
    CanRxMsg rx_msg;

    if (id >= HAL_CAN_MAX || msg == NULL)
    {
        return -1;
    }

    /* 等待接收 */
    uint32_t timeout = timeout_ms;
    while (CAN_MessagePending(CAN1, CAN_FIFO0) == 0)
    {
        if (timeout == 0)
        {
            return -1;
        }
        hal_delay_ms(1);
        timeout--;
    }

    /* 接收消息 */
    CAN_Receive(CAN1, CAN_FIFO0, &rx_msg);

    /* 转换消息 */
    if (rx_msg.IDE == CAN_Id_Extended)
    {
        msg->id = rx_msg.ExtId;
        msg->ide = 1;
    }
    else
    {
        msg->id = rx_msg.StdId;
        msg->ide = 0;
    }

    msg->rtr = (rx_msg.RTR == CAN_RTR_Remote) ? 1 : 0;
    msg->dlc = rx_msg.DLC;

    for (uint8_t i = 0; i < msg->dlc; i++)
    {
        msg->data[i] = rx_msg.Data[i];
    }

    return 0;
}

int hal_can_set_filter(hal_can_id_t id, uint32_t id_mask, uint32_t mask)
{
    CAN_FilterInitTypeDef filter_init;

    if (id >= HAL_CAN_MAX)
    {
        return -1;
    }

    filter_init.CAN_FilterNumber = 0;
    filter_init.CAN_FilterMode = CAN_FilterMode_IdMask;
    filter_init.CAN_FilterScale = CAN_FilterScale_32bit;
    filter_init.CAN_FilterIdHigh = (uint16_t)(id_mask >> 16);
    filter_init.CAN_FilterIdLow = (uint16_t)(id_mask & 0xFFFF);
    filter_init.CAN_FilterMaskIdHigh = (uint16_t)(mask >> 16);
    filter_init.CAN_FilterMaskIdLow = (uint16_t)(mask & 0xFFFF);
    filter_init.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    filter_init.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&filter_init);

    return 0;
}

int hal_can_set_callback(hal_can_id_t id, void (*callback)(hal_can_msg_t *msg))
{
    if (id >= HAL_CAN_MAX)
    {
        return -1;
    }

    can_callback = callback;

    return 0;
}
