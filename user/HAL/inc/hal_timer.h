/**
 * @file hal_timer.h
 * @brief 定时器硬件抽象层接口定义
 *
 * 提供统一的定时器操作接口，与具体芯片无关。
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 定时器ID定义
 */
typedef enum {
    HAL_TIMER_1 = 0,
    HAL_TIMER_2,
    HAL_TIMER_3,
    HAL_TIMER_4,
    HAL_TIMER_5,
    HAL_TIMER_6,
    HAL_TIMER_7,
    HAL_TIMER_8,
    HAL_TIMER_MAX
} hal_timer_id_t;

/**
 * @brief 定时器模式
 */
typedef enum {
    HAL_TIMER_MODE_UP = 0,      /**< 向上计数 */
    HAL_TIMER_MODE_DOWN,        /**< 向下计数 */
    HAL_TIMER_MODE_CENTER       /**< 中心对齐 */
} hal_timer_mode_t;

/**
 * @brief 定时器回调函数类型
 */
typedef void (*hal_timer_callback_t)(void);

/**
 * @brief 定时器配置结构体
 */
typedef struct {
    hal_timer_id_t   id;            /**< 定时器ID */
    hal_timer_mode_t mode;          /**< 计数模式 */
    uint32_t         prescaler;     /**< 预分频器 */
    uint32_t         period;        /**< 周期 */
    bool             auto_reload;   /**< 自动重载 */
    hal_timer_callback_t callback;  /**< 中断回调函数 */
} hal_timer_config_t;

/**
 * @brief 初始化定时器
 * @param config 定时器配置
 * @return 0成功，其他失败
 */
int hal_timer_init(const hal_timer_config_t *config);

/**
 * @brief 去初始化定时器
 * @param id 定时器ID
 * @return 0成功，其他失败
 */
int hal_timer_deinit(hal_timer_id_t id);

/**
 * @brief 启动定时器
 * @param id 定时器ID
 * @return 0成功，其他失败
 */
int hal_timer_start(hal_timer_id_t id);

/**
 * @brief 停止定时器
 * @param id 定时器ID
 * @return 0成功，其他失败
 */
int hal_timer_stop(hal_timer_id_t id);

/**
 * @brief 获取定时器计数值
 * @param id 定时器ID
 * @return 计数值
 */
uint32_t hal_timer_get_count(hal_timer_id_t id);

/**
 * @brief 设置定时器周期
 * @param id 定时器ID
 * @param period 周期
 * @return 0成功，其他失败
 */
int hal_timer_set_period(hal_timer_id_t id, uint32_t period);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIMER_H */
