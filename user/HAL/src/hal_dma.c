/**
 * @file hal_dma.c
 * @brief DMA硬件抽象层CH32V30x实现
 */

#include "hal_dma.h"
#include "ch32v30x.h"
#include <stddef.h>

static DMA_Channel_TypeDef * const dma_channel_map[] = {
    DMA1_Channel1, DMA1_Channel2, DMA1_Channel3, DMA1_Channel4,
    DMA1_Channel5, DMA1_Channel6, DMA1_Channel7
};

static void (*dma_callback[HAL_DMA_CH_MAX])(void) = {NULL};

int hal_dma_init(const hal_dma_config_t *config)
{
    DMA_InitTypeDef dma_init;

    if (config == NULL || config->channel >= HAL_DMA_CH_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* 配置DMA */
    dma_init.DMA_PeripheralBaseAddr = config->src_addr;
    dma_init.DMA_MemoryBaseAddr = config->dst_addr;
    dma_init.DMA_DIR = (config->direction == HAL_DMA_DIR_PERIPH_TO_MEM) ?
                       DMA_DIR_PeripheralSRC : DMA_DIR_PeripheralDST;
    dma_init.DMA_BufferSize = config->data_length;
    dma_init.DMA_PeripheralInc = config->src_inc ? DMA_PeripheralInc_Enable : DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = config->dst_inc ? DMA_MemoryInc_Enable : DMA_MemoryInc_Disable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode = DMA_Mode_Normal;
    dma_init.DMA_Priority = DMA_Priority_Medium;
    dma_init.DMA_M2M = (config->direction == HAL_DMA_DIR_MEM_TO_MEM) ?
                       DMA_M2M_Enable : DMA_M2M_Disable;

    DMA_Init(dma_channel_map[config->channel], &dma_init);

    /* 保存回调函数 */
    dma_callback[config->channel] = NULL;

    return 0;
}

int hal_dma_deinit(hal_dma_channel_t channel)
{
    if (channel >= HAL_DMA_CH_MAX)
    {
        return -1;
    }

    DMA_DeInit(dma_channel_map[channel]);
    dma_callback[channel] = NULL;

    return 0;
}

int hal_dma_start(hal_dma_channel_t channel)
{
    if (channel >= HAL_DMA_CH_MAX)
    {
        return -1;
    }

    DMA_Cmd(dma_channel_map[channel], ENABLE);

    return 0;
}

int hal_dma_stop(hal_dma_channel_t channel)
{
    if (channel >= HAL_DMA_CH_MAX)
    {
        return -1;
    }

    DMA_Cmd(dma_channel_map[channel], DISABLE);

    return 0;
}

int hal_dma_set_callback(hal_dma_channel_t channel, void (*callback)(void))
{
    if (channel >= HAL_DMA_CH_MAX)
    {
        return -1;
    }

    dma_callback[channel] = callback;

    return 0;
}
