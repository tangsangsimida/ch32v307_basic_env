/**
 * @file hal_adc.h
 * @brief ADC硬件抽象层接口定义
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ADC端口定义
 */
typedef enum {
    HAL_ADC_1 = 0,
    HAL_ADC_2,
    HAL_ADC_MAX
} hal_adc_id_t;

/**
 * @brief ADC通道定义
 */
typedef enum {
    HAL_ADC_CH0 = 0,
    HAL_ADC_CH1,
    HAL_ADC_CH2,
    HAL_ADC_CH3,
    HAL_ADC_CH4,
    HAL_ADC_CH5,
    HAL_ADC_CH6,
    HAL_ADC_CH7,
    HAL_ADC_CH8,
    HAL_ADC_CH9,
    HAL_ADC_CH_MAX
} hal_adc_channel_t;

/**
 * @brief ADC配置结构体
 */
typedef struct {
    hal_adc_id_t id;
    uint32_t     sample_time;
} hal_adc_config_t;

int hal_adc_init(const hal_adc_config_t *config);
int hal_adc_deinit(hal_adc_id_t id);
uint16_t hal_adc_read(hal_adc_id_t id, hal_adc_channel_t channel);
uint16_t hal_adc_read_average(hal_adc_id_t id, hal_adc_channel_t channel, uint8_t times);
float hal_adc_to_voltage(uint16_t adc_value, float vref);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ADC_H */
