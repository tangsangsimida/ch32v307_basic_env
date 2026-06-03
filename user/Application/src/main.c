/**
 * @file main.c
 * @brief LED闪烁示例（使用HAL接口）
 *
 * 使用硬件抽象层接口实现LED闪烁，与具体芯片无关。
 * 更换芯片时只需实现HAL层，此文件无需修改。
 */

#include "hal.h"

/* LED配置 */
#define LED_PORT       HAL_GPIO_PORT_A
#define LED_PIN        HAL_GPIO_PIN_0
#define BLINK_DELAY    500  /* 毫秒 */

/**
 * @brief 初始化LED
 */
static void led_init(void)
{
    hal_gpio_config_t config = {
        .port = LED_PORT,
        .pin = LED_PIN,
        .mode = HAL_GPIO_MODE_OUTPUT_PP,
        .speed = HAL_GPIO_SPEED_HIGH
    };

    hal_gpio_init(&config);

    /* 默认关闭LED（低电平点亮，高电平关闭） */
    hal_gpio_write(LED_PORT, LED_PIN, HAL_GPIO_LEVEL_HIGH);
}

/**
 * @brief 主函数
 */
int main(void)
{
    /* 初始化HAL */
    hal_init();

    /* 打印系统信息 */
    printf("SystemClk: %ld Hz\r\n", hal_system_get_clock());
    printf("ChipID:    %08lx\r\n", hal_get_chip_id());
    printf("LED Blink Demo (HAL)\r\n");

    /* 初始化LED */
    led_init();

    /* 主循环 */
    while (1)
    {
        /* 翻转LED */
        hal_gpio_toggle(LED_PORT, LED_PIN);

        /* 延时 */
        hal_delay_ms(BLINK_DELAY);
    }
}
