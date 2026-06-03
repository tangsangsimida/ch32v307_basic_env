/**
 * @file hal_timer.c
 * @brief 定时器硬件抽象层CH32V30x实现
 */

#include "hal_timer.h"
#include "ch32v30x.h"
#include <stddef.h>

/**
 * @brief 定时器端口映射表
 */
static TIM_TypeDef * const timer_port_map[] = {
    TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8
};

/**
 * @brief 定时器时钟映射表
 */
static const uint32_t timer_clk_map[] = {
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
 * @brief 定时器回调函数表
 */
static hal_timer_callback_t timer_callback[HAL_TIMER_MAX] = {NULL};

/**
 * @brief 初始化定时器
 */
int hal_timer_init(const hal_timer_config_t *config)
{
    TIM_TimeBaseInitTypeDef timer_init;

    if (config == NULL || config->id >= HAL_TIMER_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    if (config->id == HAL_TIMER_1 || config->id == HAL_TIMER_8)
    {
        RCC_APB2PeriphClockCmd(timer_clk_map[config->id], ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(timer_clk_map[config->id], ENABLE);
    }

    /* 配置定时器 */
    timer_init.TIM_Prescaler = config->prescaler;
    timer_init.TIM_Period = config->period;

    switch (config->mode)
    {
        case HAL_TIMER_MODE_UP:
            timer_init.TIM_CounterMode = TIM_CounterMode_Up;
            break;
        case HAL_TIMER_MODE_DOWN:
            timer_init.TIM_CounterMode = TIM_CounterMode_Down;
            break;
        case HAL_TIMER_MODE_CENTER:
            timer_init.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
            break;
        default:
            timer_init.TIM_CounterMode = TIM_CounterMode_Up;
            break;
    }

    timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
    timer_init.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(timer_port_map[config->id], &timer_init);

    /* 保存回调函数 */
    timer_callback[config->id] = config->callback;

    return 0;
}

/**
 * @brief 去初始化定时器
 */
int hal_timer_deinit(hal_timer_id_t id)
{
    if (id >= HAL_TIMER_MAX)
    {
        return -1;
    }

    TIM_Cmd(timer_port_map[id], DISABLE);
    timer_callback[id] = NULL;

    return 0;
}

/**
 * @brief 启动定时器
 */
int hal_timer_start(hal_timer_id_t id)
{
    if (id >= HAL_TIMER_MAX)
    {
        return -1;
    }

    TIM_Cmd(timer_port_map[id], ENABLE);

    return 0;
}

/**
 * @brief 停止定时器
 */
int hal_timer_stop(hal_timer_id_t id)
{
    if (id >= HAL_TIMER_MAX)
    {
        return -1;
    }

    TIM_Cmd(timer_port_map[id], DISABLE);

    return 0;
}

/**
 * @brief 获取定时器计数值
 */
uint32_t hal_timer_get_count(hal_timer_id_t id)
{
    if (id >= HAL_TIMER_MAX)
    {
        return 0;
    }

    return TIM_GetCounter(timer_port_map[id]);
}

/**
 * @brief 设置定时器周期
 */
int hal_timer_set_period(hal_timer_id_t id, uint32_t period)
{
    if (id >= HAL_TIMER_MAX)
    {
        return -1;
    }

    TIM_SetAutoreload(timer_port_map[id], period);

    return 0;
}
