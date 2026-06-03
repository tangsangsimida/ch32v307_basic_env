/**
 * @file hal.c
 * @brief 硬件抽象层初始化
 */

#include "hal.h"

/**
 * @brief 初始化所有HAL模块
 */
void hal_init(void)
{
    /* 初始化系统 */
    hal_system_init();
}
