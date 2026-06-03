/**
 * @file hal_pwm.c
 * @brief PWM硬件抽象层CH32V30x实现
 */

#include "hal_pwm.h"
#include "ch32v30x.h"
#include <stddef.h>

static TIM_TypeDef * const pwm_timer_map[] = {
    TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8
};

static const uint16_t pwm_channel_map[] = {
    TIM_Channel_1, TIM_Channel_2, TIM_Channel_3, TIM_Channel_4
};

int hal_pwm_init(const hal_pwm_config_t *config)
{
    TIM_TimeBaseInitTypeDef timer_init;
    TIM_OCInitTypeDef oc_init;

    if (config == NULL || config->channel >= HAL_PWM_CH_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    if (config->timer_id == 0 || config->timer_id == 7)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 << config->timer_id, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 << (config->timer_id - 1), ENABLE);
    }

    /* 配置定时器 */
    timer_init.TIM_Prescaler = SystemCoreClock / config->frequency_hz / 1000 - 1;
    timer_init.TIM_Period = 999;
    timer_init.TIM_CounterMode = TIM_CounterMode_Up;
    timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
    timer_init.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(pwm_timer_map[config->timer_id], &timer_init);

    /* 配置PWM */
    oc_init.TIM_OCMode = TIM_OCMode_PWM1;
    oc_init.TIM_OutputState = TIM_OutputState_Enable;
    oc_init.TIM_Pulse = config->duty_cycle * 10;
    oc_init.TIM_OCPolarity = TIM_OCPolarity_High;

    /* 根据通道调用对应的初始化函数 */
    switch (config->channel)
    {
        case HAL_PWM_CH1:
            TIM_OC1Init(pwm_timer_map[config->timer_id], &oc_init);
            break;
        case HAL_PWM_CH2:
            TIM_OC2Init(pwm_timer_map[config->timer_id], &oc_init);
            break;
        case HAL_PWM_CH3:
            TIM_OC3Init(pwm_timer_map[config->timer_id], &oc_init);
            break;
        case HAL_PWM_CH4:
            TIM_OC4Init(pwm_timer_map[config->timer_id], &oc_init);
            break;
        default:
            return -1;
    }

    return 0;
}

int hal_pwm_deinit(uint32_t timer_id, hal_pwm_channel_t channel)
{
    if (timer_id >= 8 || channel >= HAL_PWM_CH_MAX)
    {
        return -1;
    }

    TIM_Cmd(pwm_timer_map[timer_id], DISABLE);
    return 0;
}

int hal_pwm_start(uint32_t timer_id, hal_pwm_channel_t channel)
{
    if (timer_id >= 8 || channel >= HAL_PWM_CH_MAX)
    {
        return -1;
    }

    TIM_Cmd(pwm_timer_map[timer_id], ENABLE);
    return 0;
}

int hal_pwm_stop(uint32_t timer_id, hal_pwm_channel_t channel)
{
    if (timer_id >= 8 || channel >= HAL_PWM_CH_MAX)
    {
        return -1;
    }

    TIM_Cmd(pwm_timer_map[timer_id], DISABLE);
    return 0;
}

int hal_pwm_set_duty(uint32_t timer_id, hal_pwm_channel_t channel, uint8_t duty)
{
    if (timer_id >= 8 || channel >= HAL_PWM_CH_MAX || duty > 100)
    {
        return -1;
    }

    TIM_SetCompare1(pwm_timer_map[timer_id], duty * 10);
    return 0;
}

int hal_pwm_set_frequency(uint32_t timer_id, hal_pwm_channel_t channel, uint32_t freq_hz)
{
    if (timer_id >= 8 || channel >= HAL_PWM_CH_MAX)
    {
        return -1;
    }

    TIM_SetAutoreload(pwm_timer_map[timer_id], SystemCoreClock / freq_hz / 1000 - 1);
    return 0;
}
