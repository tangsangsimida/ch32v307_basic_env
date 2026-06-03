/**
 * @file hal_gpio.h
 * @brief GPIO硬件抽象层接口定义
 *
 * 提供统一的GPIO操作接口，与具体芯片无关。
 * 更换芯片时只需实现对应的hal_gpio.c即可。
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GPIO端口定义
 */
typedef enum {
    HAL_GPIO_PORT_A = 0,
    HAL_GPIO_PORT_B,
    HAL_GPIO_PORT_C,
    HAL_GPIO_PORT_D,
    HAL_GPIO_PORT_E,
    HAL_GPIO_PORT_MAX
} hal_gpio_port_t;

/**
 * @brief GPIO引脚定义
 */
typedef enum {
    HAL_GPIO_PIN_0  = (1 << 0),
    HAL_GPIO_PIN_1  = (1 << 1),
    HAL_GPIO_PIN_2  = (1 << 2),
    HAL_GPIO_PIN_3  = (1 << 3),
    HAL_GPIO_PIN_4  = (1 << 4),
    HAL_GPIO_PIN_5  = (1 << 5),
    HAL_GPIO_PIN_6  = (1 << 6),
    HAL_GPIO_PIN_7  = (1 << 7),
    HAL_GPIO_PIN_8  = (1 << 8),
    HAL_GPIO_PIN_9  = (1 << 9),
    HAL_GPIO_PIN_10 = (1 << 10),
    HAL_GPIO_PIN_11 = (1 << 11),
    HAL_GPIO_PIN_12 = (1 << 12),
    HAL_GPIO_PIN_13 = (1 << 13),
    HAL_GPIO_PIN_14 = (1 << 14),
    HAL_GPIO_PIN_15 = (1 << 15),
    HAL_GPIO_PIN_ALL = 0xFFFF
} hal_gpio_pin_t;

/**
 * @brief GPIO模式定义
 */
typedef enum {
    HAL_GPIO_MODE_INPUT,           /**< 浮空输入 */
    HAL_GPIO_MODE_INPUT_PULLUP,    /**< 上拉输入 */
    HAL_GPIO_MODE_INPUT_PULLDOWN,  /**< 下拉输入 */
    HAL_GPIO_MODE_OUTPUT_PP,       /**< 推挽输出 */
    HAL_GPIO_MODE_OUTPUT_OD,       /**< 开漏输出 */
    HAL_GPIO_MODE_AF_PP,           /**< 复用推挽 */
    HAL_GPIO_MODE_AF_OD,           /**< 复用开漏 */
    HAL_GPIO_MODE_ANALOG           /**< 模拟模式 */
} hal_gpio_mode_t;

/**
 * @brief GPIO速度定义
 */
typedef enum {
    HAL_GPIO_SPEED_LOW    = 0,     /**< 低速 */
    HAL_GPIO_SPEED_MEDIUM = 1,     /**< 中速 */
    HAL_GPIO_SPEED_HIGH   = 2,     /**< 高速 */
    HAL_GPIO_SPEED_VERY_HIGH = 3   /**< 超高速 */
} hal_gpio_speed_t;

/**
 * @brief GPIO电平定义
 */
typedef enum {
    HAL_GPIO_LEVEL_LOW = 0,        /**< 低电平 */
    HAL_GPIO_LEVEL_HIGH = 1        /**< 高电平 */
} hal_gpio_level_t;

/**
 * @brief GPIO配置结构体
 */
typedef struct {
    hal_gpio_port_t port;          /**< 端口 */
    hal_gpio_pin_t  pin;           /**< 引脚 */
    hal_gpio_mode_t mode;          /**< 模式 */
    hal_gpio_speed_t speed;        /**< 速度 */
} hal_gpio_config_t;

/**
 * @brief 初始化GPIO
 * @param config GPIO配置
 * @return 0成功，其他失败
 */
int hal_gpio_init(const hal_gpio_config_t *config);

/**
 * @brief 去初始化GPIO
 * @param port 端口
 * @param pin 引脚
 * @return 0成功，其他失败
 */
int hal_gpio_deinit(hal_gpio_port_t port, hal_gpio_pin_t pin);

/**
 * @brief 设置GPIO电平
 * @param port 端口
 * @param pin 引脚
 * @param level 电平
 */
void hal_gpio_write(hal_gpio_port_t port, hal_gpio_pin_t pin, hal_gpio_level_t level);

/**
 * @brief 读取GPIO电平
 * @param port 端口
 * @param pin 引脚
 * @return 电平值
 */
hal_gpio_level_t hal_gpio_read(hal_gpio_port_t port, hal_gpio_pin_t pin);

/**
 * @brief 翻转GPIO电平
 * @param port 端口
 * @param pin 引脚
 */
void hal_gpio_toggle(hal_gpio_port_t port, hal_gpio_pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */
