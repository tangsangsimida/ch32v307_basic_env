/**
 * @file hal_system.c
 * @brief 系统硬件抽象层CH32V30x实现
 *
 * 基于CH32V30x标准外设库实现系统HAL接口。
 * 更换芯片时需要重写此文件。
 */

#include "hal_system.h"
#include "ch32v30x.h"

/**
 * @brief 系统滴答计数器
 */
static volatile uint32_t sys_tick_count = 0;

/**
 * @brief 初始化系统
 */
void hal_system_init(void)
{
    /* 配置中断优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 更新系统时钟 */
    SystemCoreClockUpdate();

    /* 初始化延时 */
    Delay_Init();

    /* 初始化串口调试 */
    USART_Printf_Init(115200);
}

/**
 * @brief 获取系统时钟频率
 */
uint32_t hal_system_get_clock(void)
{
    return SystemCoreClock;
}

/**
 * @brief 获取系统滴答计数
 */
uint32_t hal_system_get_tick(void)
{
    return sys_tick_count;
}

/**
 * @brief 毫秒延时
 */
void hal_delay_ms(uint32_t ms)
{
    Delay_Ms(ms);
}

/**
 * @brief 微秒延时
 */
void hal_delay_us(uint32_t us)
{
    Delay_Us(us);
}

/**
 * @brief 系统复位
 */
void hal_system_reset(void)
{
    NVIC_SystemReset();
}

/**
 * @brief 禁用全局中断
 */
void hal_interrupt_disable(void)
{
    __disable_irq();
}

/**
 * @brief 使能全局中断
 */
void hal_interrupt_enable(void)
{
    __enable_irq();
}

/**
 * @brief 获取芯片ID
 */
uint32_t hal_get_chip_id(void)
{
    return DBGMCU_GetCHIPID();
}
