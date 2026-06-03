/**
 * @file hal.h
 * @brief 硬件抽象层总头文件
 *
 * 包含此文件即可使用所有HAL接口。
 */

#ifndef HAL_H
#define HAL_H

#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "hal_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化所有HAL模块
 */
void hal_init(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */
