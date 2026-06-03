/**
 * @file hal_wdg.h
 * @brief 看门狗硬件抽象层接口定义
 */

#ifndef HAL_WDG_H
#define HAL_WDG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 看门狗类型
 */
typedef enum {
    HAL_WDG_IWDG = 0,  /* 独立看门狗 */
    HAL_WDG_WWDG        /* 窗口看门狗 */
} hal_wdg_type_t;

/**
 * @brief 看门狗配置结构体
 */
typedef struct {
    hal_wdg_type_t type;
    uint32_t timeout_ms;
} hal_wdg_config_t;

int hal_wdg_init(const hal_wdg_config_t *config);
int hal_wdg_deinit(void);
int hal_wdg_feed(void);
int hal_wdg_enable(void);
int hal_wdg_disable(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_WDG_H */
