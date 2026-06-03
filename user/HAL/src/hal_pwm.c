/**
 * @file hal_pwm.c
 * @brief PWM硬件抽象层CH32V30x实现
 */

#include "hal_pwm.h"
#include "ch32v30x.h"
#include <stddef.h>

/**
 * @brief 定时器端口映射表
 */
static TIM_TypeDef * const pwm_timer_map[] = {
    TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8
};

/**
 * @brief 定时器时钟映射表
 */
static const uint32_t pwm_timer_clk[] = {
    RCC_APB2Periph_TIM1,
    RCC_APB1Periph_TIM2,
    RCC_APB1Periph_TIM3,
    RCC_APB1Periph_TIM4,
    RCC_APB1Periph_TIM5,
    RCC_APB1Periph_TIM6,
    RCC_APB1Periph_TIM7,
    RCC_APB2Periph_TIM8
};

/**
 * @brief 定时器时钟类型（0=APB2, 1=APB1）
 */
static const uint8_t pwm_timer_clk_type[] = {
    0, 1, 1, 1, 1, 1, 1, 0
};

int hal_pwm_init(const hal_pwm_config_t *config)
{
    TIM_TimeBaseInitTypeDef timer_init;
    TIM_OCInitTypeDef oc_init;

    if (config == NULL || config->timer_id >= 8 || config->channel >= HAL_PWM_CH_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    if (pwm_timer_clk_type[config->timer_id] == 0)
    {
        RCC_APB2PeriphClockCmd(pwm_timer_clk[config->timer_id], ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(pwm_timer_clk[config->timer_id], ENABLE);
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
            TIM_OC1PreloadConfig(pwm_timer_map[config->timer_id], TIM_OCPreload_Enable);
            break;
        case HAL_PWM_CH2:
            TIM_OC2Init(pwm_timer_map[config->timer_id], &oc_init);
            TIM_OC2PreloadConfig(pwm_timer_map[config->timer_id], TIM_OCPreload_Enable);
            break;
        case HAL_PWM_CH3:
            TIM_OC3Init(pwm_timer_map[config->timer_id], &oc_init);
            TIM_OC3PreloadConfig(pwm_timer_map[config->timer_id], TIM_OCPreload_Enable);
            break;
        case HAL_PWM_CH4:
            TIM_OC4Init(pwm_timer_map[config->timer_id], &oc_init);
            TIM_OC4PreloadConfig(pwm_timer_map[config->timer_id], TIM_OCPreload_Enable);
            break;
        default:
            return -1;
    }

    TIM_ARRPreloadConfig(pwm_timer_map[config->timer_id], ENABLE);

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

    /* 使能PWM输出（TIM1/TIM8需要额外使能） */
    if (timer_id == 0 || timer_id == 7)
    {
        TIM_CtrlPWMOutputs(pwm_timer_map[timer_id], ENABLE);
    }

    return 0;
}

int hal_pwm_stop(uint32_t timer_id, hal_pwm_channel_t channel)
{
    if (timer_id >= 8 || channel >= HAL_PWM_CH_MAX)
    {
        return -1;
    }

    /* 禁用PWM输出（TIM1/TIM8需要额外禁用） */
    if (timer_id == 0 || timer_id == 7)
    {
        TIM_CtrlPWMOutputs(pwm_timer_map[timer_id], DISABLE);
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

    uint16_t pulse = duty * 10;

    switch (channel)
    {
        case HAL_PWM_CH1:
            TIM_SetCompare1(pwm_timer_map[timer_id], pulse);
            break;
        case HAL_PWM_CH2:
            TIM_SetCompare2(pwm_timer_map[timer_id], pulse);
            break;
        case HAL_PWM_CH3:
            TIM_SetCompare3(pwm_timer_map[timer_id], pulse);
            break;
        case HAL_PWM_CH4:
            TIM_SetCompare4(pwm_timer_map[timer_id], pulse);
            break;
        default:
            return -1;
    }

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
