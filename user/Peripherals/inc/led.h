/**
 * @file led.h
 * @brief LED驱动接口定义
 */

#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED标识
 */
typedef enum {
    LED1 = 0,
    LED2,
    LED_MAX
} led_id_t;

/**
 * @brief 初始化所有LED引脚
 */
void led_init(void);

/**
 * @brief 点亮LED
 * @param id LED标识
 */
void led_on(led_id_t id);

/**
 * @brief 熄灭LED
 * @param id LED标识
 */
void led_off(led_id_t id);

/**
 * @brief 翻转LED
 * @param id LED标识
 */
void led_toggle(led_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
