/**
 * @file hal_uart.c
 * @brief UART硬件抽象层CH32V30x实现
 *
 * 基于CH32V30x标准外设库实现UART HAL接口。
 * 更换芯片时需要重写此文件。
 */

#include "hal_uart.h"
#include "hal_system.h"
#include "ch32v30x.h"
#include <stddef.h>
#include <string.h>

/**
 * @brief UART端口映射表
 */
static USART_TypeDef * const uart_port_map[HAL_UART_MAX] = {
    USART1,
    USART2,
    USART3,
    UART4,
    UART5
};

/**
 * @brief UART时钟映射表
 */
static const uint32_t uart_clk_map[HAL_UART_MAX] = {
    RCC_APB2Periph_USART1,
    RCC_APB1Periph_USART2,
    RCC_APB1Periph_USART3,
    RCC_APB1Periph_UART4,
    RCC_APB1Periph_UART5
};

/**
 * @brief UART GPIO映射结构
 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t tx_pin;
    uint16_t rx_pin;
    uint32_t gpio_clk;
} uart_gpio_map_t;

/**
 * @brief UART GPIO映射表
 */
static const uart_gpio_map_t uart_gpio_map[HAL_UART_MAX] = {
    { GPIOA, GPIO_Pin_9,  GPIO_Pin_10, RCC_APB2Periph_GPIOA },  /* USART1 */
    { GPIOA, GPIO_Pin_2,  GPIO_Pin_3,  RCC_APB2Periph_GPIOA },  /* USART2 */
    { GPIOB, GPIO_Pin_10, GPIO_Pin_11, RCC_APB2Periph_GPIOB },  /* USART3 */
    { GPIOC, GPIO_Pin_10, GPIO_Pin_11, RCC_APB2Periph_GPIOC },  /* UART4  */
    { GPIOC, GPIO_Pin_12, GPIO_Pin_2,  RCC_APB2Periph_GPIOC }   /* UART5  */
};

/**
 * @brief 初始化UART
 */
int hal_uart_init(const hal_uart_config_t *config)
{
    USART_InitTypeDef usart_init;
    GPIO_InitTypeDef gpio_init;

    if (config == NULL || config->id >= HAL_UART_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    if (config->id == HAL_UART_1)
    {
        RCC_APB2PeriphClockCmd(uart_clk_map[config->id], ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(uart_clk_map[config->id], ENABLE);
    }

    RCC_APB2PeriphClockCmd(uart_gpio_map[config->id].gpio_clk, ENABLE);

    /* 配置TX引脚 */
    gpio_init.GPIO_Pin = uart_gpio_map[config->id].tx_pin;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(uart_gpio_map[config->id].port, &gpio_init);

    /* 配置RX引脚 */
    gpio_init.GPIO_Pin = uart_gpio_map[config->id].rx_pin;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(uart_gpio_map[config->id].port, &gpio_init);

    /* 配置UART参数 */
    usart_init.USART_BaudRate = config->baudrate;

    switch (config->databits)
    {
        case HAL_UART_DATA_8:
            usart_init.USART_WordLength = USART_WordLength_8b;
            break;
        case HAL_UART_DATA_9:
            usart_init.USART_WordLength = USART_WordLength_9b;
            break;
        default:
            usart_init.USART_WordLength = USART_WordLength_8b;
            break;
    }

    switch (config->stopbits)
    {
        case HAL_UART_STOP_1:
            usart_init.USART_StopBits = USART_StopBits_1;
            break;
        case HAL_UART_STOP_2:
            usart_init.USART_StopBits = USART_StopBits_2;
            break;
        default:
            usart_init.USART_StopBits = USART_StopBits_1;
            break;
    }

    switch (config->parity)
    {
        case HAL_UART_PARITY_NONE:
            usart_init.USART_Parity = USART_Parity_No;
            break;
        case HAL_UART_PARITY_EVEN:
            usart_init.USART_Parity = USART_Parity_Even;
            break;
        case HAL_UART_PARITY_ODD:
            usart_init.USART_Parity = USART_Parity_Odd;
            break;
        default:
            usart_init.USART_Parity = USART_Parity_No;
            break;
    }

    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(uart_port_map[config->id], &usart_init);

    /* 使能UART */
    USART_Cmd(uart_port_map[config->id], ENABLE);

    return 0;
}

/**
 * @brief 去初始化UART
 */
int hal_uart_deinit(hal_uart_id_t id)
{
    if (id >= HAL_UART_MAX)
    {
        return -1;
    }

    USART_Cmd(uart_port_map[id], DISABLE);

    return 0;
}

/**
 * @brief 发送数据
 */
int hal_uart_send(hal_uart_id_t id, const uint8_t *data, size_t len)
{
    if (id >= HAL_UART_MAX || data == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < len; i++)
    {
        while (USART_GetFlagStatus(uart_port_map[id], USART_FLAG_TC) == RESET)
            ;
        USART_SendData(uart_port_map[id], data[i]);
    }

    return (int)len;
}

/**
 * @brief 接收数据
 */
int hal_uart_recv(hal_uart_id_t id, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (id >= HAL_UART_MAX || data == NULL)
    {
        return -1;
    }

    /* 简单的轮询接收实现 */
    for (size_t i = 0; i < len; i++)
    {
        uint32_t timeout = timeout_ms;

        while (USART_GetFlagStatus(uart_port_map[id], USART_FLAG_RXNE) == RESET)
        {
            if (timeout == 0)
            {
                return (int)i;
            }
            hal_delay_ms(1);
            timeout--;
        }

        data[i] = (uint8_t)USART_ReceiveData(uart_port_map[id]);
    }

    return (int)len;
}

/**
 * @brief 发送字符串
 */
int hal_uart_send_string(hal_uart_id_t id, const char *str)
{
    if (str == NULL)
    {
        return -1;
    }

    return hal_uart_send(id, (const uint8_t *)str, strlen(str));
}

/**
 * @brief 获取接收缓冲区中的数据长度
 */
int hal_uart_available(hal_uart_id_t id)
{
    if (id >= HAL_UART_MAX)
    {
        return 0;
    }

    if (USART_GetFlagStatus(uart_port_map[id], USART_FLAG_RXNE) == SET)
    {
        return 1;
    }

    return 0;
}
