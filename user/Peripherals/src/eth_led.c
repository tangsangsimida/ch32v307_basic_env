/**
 * @file eth_led.c
 * @brief 以太网状态 LED 驱动实现
 *
 * 硬件连接（低电平点亮）：
 *   ELED1 - PC10 - Link 状态
 *   ELED2 - PC11 - Activity 活动
 */

#include "eth_led.h"
#include "hal_gpio.h"

/* Activity LED 亮灯持续时间（主循环周期数，10ms/次 → 5 = 50ms） */
#define ACT_LED_TICKS  5

static const hal_gpio_config_t eth_led_config[] = {
    /* ELED1 - PC10 - Link */
    {
        .port  = HAL_GPIO_PORT_C,
        .pin   = HAL_GPIO_PIN_10,
        .mode  = HAL_GPIO_MODE_OUTPUT_PP,
        .speed = HAL_GPIO_SPEED_LOW,
    },
    /* ELED2 - PC11 - Activity */
    {
        .port  = HAL_GPIO_PORT_C,
        .pin   = HAL_GPIO_PIN_11,
        .mode  = HAL_GPIO_MODE_OUTPUT_PP,
        .speed = HAL_GPIO_SPEED_LOW,
    },
};

#define ETH_LED_LINK  0
#define ETH_LED_ACT   1

static volatile uint8_t act_led_counter = 0;

/* 低电平点亮：ON=LOW, OFF=HIGH */
static void eth_led_set(int index, int on)
{
    hal_gpio_write(eth_led_config[index].port,
                   eth_led_config[index].pin,
                   on ? HAL_GPIO_LEVEL_LOW : HAL_GPIO_LEVEL_HIGH);
}

void eth_led_init(void)
{
    hal_gpio_init(&eth_led_config[ETH_LED_LINK]);
    hal_gpio_init(&eth_led_config[ETH_LED_ACT]);

    /* 初始状态：熄灭 */
    eth_led_set(ETH_LED_LINK, 0);
    eth_led_set(ETH_LED_ACT, 0);
    act_led_counter = 0;
}

void eth_led_link_update(int link_up)
{
    eth_led_set(ETH_LED_LINK, link_up);
}

void eth_led_activity_trigger(void)
{
    eth_led_set(ETH_LED_ACT, 1);
    act_led_counter = ACT_LED_TICKS;
}

void eth_led_tick(void)
{
    if (act_led_counter > 0)
    {
        act_led_counter--;
        if (act_led_counter == 0)
        {
            eth_led_set(ETH_LED_ACT, 0);
        }
    }
}
