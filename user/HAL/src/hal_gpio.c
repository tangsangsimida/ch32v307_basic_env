/**
 * @file hal_gpio.c
 * @brief GPIO硬件抽象层CH32V30x实现
 *
 * 基于CH32V30x标准外设库实现GPIO HAL接口。
 * 更换芯片时需要重写此文件。
 */

#include "hal_gpio.h"
#include "ch32v30x.h"
#include <stddef.h>

/**
 * @brief GPIO端口映射表
 */
static GPIO_TypeDef * const gpio_port_map[] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE
};

/**
 * @brief GPIO时钟映射表
 */
static const uint32_t gpio_clk_map[] = {
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOD,
    RCC_APB2Periph_GPIOE
};

/**
 * @brief GPIO端口数量
 */
#define GPIO_PORT_COUNT (sizeof(gpio_port_map) / sizeof(gpio_port_map[0]))

/**
 * @brief 初始化GPIO
 */
int hal_gpio_init(const hal_gpio_config_t *config)
{
    GPIO_InitTypeDef gpio_init;

    if (config == NULL || config->port >= HAL_GPIO_PORT_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    RCC_APB2PeriphClockCmd(gpio_clk_map[config->port], ENABLE);

    /* 配置引脚 */
    gpio_init.GPIO_Pin = (uint16_t)config->pin;

    switch (config->mode)
    {
        case HAL_GPIO_MODE_INPUT:
            gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
        case HAL_GPIO_MODE_INPUT_PULLUP:
            gpio_init.GPIO_Mode = GPIO_Mode_IPU;
            break;
        case HAL_GPIO_MODE_INPUT_PULLDOWN:
            gpio_init.GPIO_Mode = GPIO_Mode_IPD;
            break;
        case HAL_GPIO_MODE_OUTPUT_PP:
            gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
            break;
        case HAL_GPIO_MODE_OUTPUT_OD:
            gpio_init.GPIO_Mode = GPIO_Mode_Out_OD;
            break;
        case HAL_GPIO_MODE_AF_PP:
            gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
            break;
        case HAL_GPIO_MODE_AF_OD:
            gpio_init.GPIO_Mode = GPIO_Mode_AF_OD;
            break;
        case HAL_GPIO_MODE_ANALOG:
            gpio_init.GPIO_Mode = GPIO_Mode_AIN;
            break;
        default:
            return -1;
    }

    switch (config->speed)
    {
        case HAL_GPIO_SPEED_LOW:
            gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
            break;
        case HAL_GPIO_SPEED_MEDIUM:
            gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
            break;
        case HAL_GPIO_SPEED_HIGH:
        case HAL_GPIO_SPEED_VERY_HIGH:
            gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
            break;
        default:
            gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
            break;
    }

    GPIO_Init(gpio_port_map[config->port], &gpio_init);

    return 0;
}

/**
 * @brief 去初始化GPIO
 */
int hal_gpio_deinit(hal_gpio_port_t port, hal_gpio_pin_t pin)
{
    if (port >= HAL_GPIO_PORT_MAX)
    {
        return -1;
    }

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = (uint16_t)pin;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(gpio_port_map[port], &gpio_init);

    return 0;
}

/**
 * @brief 设置GPIO电平
 */
void hal_gpio_write(hal_gpio_port_t port, hal_gpio_pin_t pin, hal_gpio_level_t level)
{
    if (port >= HAL_GPIO_PORT_MAX)
    {
        return;
    }

    if (level == HAL_GPIO_LEVEL_HIGH)
    {
        GPIO_SetBits(gpio_port_map[port], (uint16_t)pin);
    }
    else
    {
        GPIO_ResetBits(gpio_port_map[port], (uint16_t)pin);
    }
}

/**
 * @brief 读取GPIO电平
 */
hal_gpio_level_t hal_gpio_read(hal_gpio_port_t port, hal_gpio_pin_t pin)
{
    if (port >= HAL_GPIO_PORT_MAX)
    {
        return HAL_GPIO_LEVEL_LOW;
    }

    if (GPIO_ReadInputDataBit(gpio_port_map[port], (uint16_t)pin) == Bit_SET)
    {
        return HAL_GPIO_LEVEL_HIGH;
    }

    return HAL_GPIO_LEVEL_LOW;
}

/**
 * @brief 翻转GPIO电平
 */
void hal_gpio_toggle(hal_gpio_port_t port, hal_gpio_pin_t pin)
{
    if (port >= HAL_GPIO_PORT_MAX)
    {
        return;
    }

    if (GPIO_ReadOutputDataBit(gpio_port_map[port], (uint16_t)pin) == Bit_SET)
    {
        GPIO_ResetBits(gpio_port_map[port], (uint16_t)pin);
    }
    else
    {
        GPIO_SetBits(gpio_port_map[port], (uint16_t)pin);
    }
}
