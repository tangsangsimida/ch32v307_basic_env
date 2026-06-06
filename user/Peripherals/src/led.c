/**
 * @file led.c
 * @brief LED驱动实现
 *
 * 硬件连接：
 *   LED1 - PE8
 *   LED2 - PE9
 */

#include "led.h"
#include "hal_gpio.h"

/**
 * @brief LED引脚配置表
 */
static const hal_gpio_config_t led_config[LED_MAX] = {
    [LED1] = {
        .port  = HAL_GPIO_PORT_E,
        .pin   = HAL_GPIO_PIN_8,
        .mode  = HAL_GPIO_MODE_OUTPUT_PP,
        .speed = HAL_GPIO_SPEED_LOW,
    },
    [LED2] = {
        .port  = HAL_GPIO_PORT_E,
        .pin   = HAL_GPIO_PIN_9,
        .mode  = HAL_GPIO_MODE_OUTPUT_PP,
        .speed = HAL_GPIO_SPEED_LOW,
    },
};

void led_init(void)
{
    for (int i = 0; i < LED_MAX; i++)
    {
        hal_gpio_init(&led_config[i]);
        hal_gpio_write(led_config[i].port, led_config[i].pin, HAL_GPIO_LEVEL_LOW);
    }
}

void led_on(led_id_t id)
{
    if (id < LED_MAX)
    {
        hal_gpio_write(led_config[id].port, led_config[id].pin, HAL_GPIO_LEVEL_HIGH);
    }
}

void led_off(led_id_t id)
{
    if (id < LED_MAX)
    {
        hal_gpio_write(led_config[id].port, led_config[id].pin, HAL_GPIO_LEVEL_LOW);
    }
}

void led_toggle(led_id_t id)
{
    if (id < LED_MAX)
    {
        hal_gpio_toggle(led_config[id].port, led_config[id].pin);
    }
}
