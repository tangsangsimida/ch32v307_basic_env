/**
 * @file hal_dma.h
 * @brief DMA硬件抽象层接口定义
 */

#ifndef HAL_DMA_H
#define HAL_DMA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DMA通道定义
 */
typedef enum {
    HAL_DMA_CH1 = 0,
    HAL_DMA_CH2,
    HAL_DMA_CH3,
    HAL_DMA_CH4,
    HAL_DMA_CH5,
    HAL_DMA_CH6,
    HAL_DMA_CH7,
    HAL_DMA_CH_MAX
} hal_dma_channel_t;

/**
 * @brief DMA方向
 */
typedef enum {
    HAL_DMA_DIR_PERIPH_TO_MEM = 0,
    HAL_DMA_DIR_MEM_TO_PERIPH,
    HAL_DMA_DIR_MEM_TO_MEM
} hal_dma_dir_t;

/**
 * @brief DMA配置结构体
 */
typedef struct {
    hal_dma_channel_t channel;
    hal_dma_dir_t direction;
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t data_length;
    uint8_t src_inc;
    uint8_t dst_inc;
} hal_dma_config_t;

int hal_dma_init(const hal_dma_config_t *config);
int hal_dma_deinit(hal_dma_channel_t channel);
int hal_dma_start(hal_dma_channel_t channel);
int hal_dma_stop(hal_dma_channel_t channel);
int hal_dma_set_callback(hal_dma_channel_t channel, void (*callback)(void));

#ifdef __cplusplus
}
#endif

#endif /* HAL_DMA_H */
