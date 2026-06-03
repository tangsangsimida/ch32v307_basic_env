/**
 * @file hal_adc.c
 * @brief ADC硬件抽象层CH32V30x实现
 */

#include "hal_adc.h"
#include "ch32v30x.h"
#include <stddef.h>

static ADC_TypeDef * const adc_port_map[] = { ADC1, ADC2 };

int hal_adc_init(const hal_adc_config_t *config)
{
    ADC_InitTypeDef adc_init;

    if (config == NULL || config->id >= HAL_ADC_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* 配置ADC */
    adc_init.ADC_Mode = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode = DISABLE;
    adc_init.ADC_ContinuousConvMode = DISABLE;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel = 1;

    ADC_Init(adc_port_map[config->id], &adc_init);
    ADC_Cmd(adc_port_map[config->id], ENABLE);

    /* 校准 */
    ADC_ResetCalibration(adc_port_map[config->id]);
    while (ADC_GetResetCalibrationStatus(adc_port_map[config->id]))
        ;
    ADC_StartCalibration(adc_port_map[config->id]);
    while (ADC_GetCalibrationStatus(adc_port_map[config->id]))
        ;

    return 0;
}

int hal_adc_deinit(hal_adc_id_t id)
{
    if (id >= HAL_ADC_MAX)
    {
        return -1;
    }

    ADC_Cmd(adc_port_map[id], DISABLE);
    return 0;
}

uint16_t hal_adc_read(hal_adc_id_t id, hal_adc_channel_t channel)
{
    if (id >= HAL_ADC_MAX || channel >= HAL_ADC_CH_MAX)
    {
        return 0;
    }

    /* 配置通道 */
    ADC_RegularChannelConfig(adc_port_map[id], channel, 1, ADC_SampleTime_239Cycles5);

    /* 开始转换 */
    ADC_SoftwareStartConvCmd(adc_port_map[id], ENABLE);

    /* 等待转换完成 */
    while (!ADC_GetFlagStatus(adc_port_map[id], ADC_FLAG_EOC))
        ;

    return ADC_GetConversionValue(adc_port_map[id]);
}

uint16_t hal_adc_read_average(hal_adc_id_t id, hal_adc_channel_t channel, uint8_t times)
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < times; i++)
    {
        sum += hal_adc_read(id, channel);
    }

    return (uint16_t)(sum / times);
}

float hal_adc_to_voltage(uint16_t adc_value, float vref)
{
    return (float)adc_value * vref / 4096.0f;
}
