/**
 * @file hal_system.h
 * @brief 系统硬件抽象层接口定义
 *
 * 提供统一的系统操作接口，与具体芯片无关。
 */

#ifndef HAL_SYSTEM_H
#define HAL_SYSTEM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化系统
 *
 * 初始化时钟、中断等系统资源
 */
void hal_system_init(void);

/**
 * @brief 获取系统时钟频率
 * @return 系统时钟频率（Hz）
 */
uint32_t hal_system_get_clock(void);

/**
 * @brief 获取系统滴答计数
 * @return 滴答计数（毫秒）
 */
uint32_t hal_system_get_tick(void);

/**
 * @brief 毫秒延时
 * @param ms 延时时间（毫秒）
 */
void hal_delay_ms(uint32_t ms);

/**
 * @brief 微秒延时
 * @param us 延时时间（微秒）
 */
void hal_delay_us(uint32_t us);

/**
 * @brief 系统复位
 */
void hal_system_reset(void);

/**
 * @brief 禁用全局中断
 */
void hal_interrupt_disable(void);

/**
 * @brief 使能全局中断
 */
void hal_interrupt_enable(void);

/**
 * @brief 获取芯片ID
 * @return 芯片ID
 */
uint32_t hal_get_chip_id(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SYSTEM_H */
