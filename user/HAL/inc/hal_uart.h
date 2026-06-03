/**
 * @file hal_uart.h
 * @brief UART硬件抽象层接口定义
 *
 * 提供统一的UART操作接口，与具体芯片无关。
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UART端口定义
 */
typedef enum {
    HAL_UART_1 = 0,    /**< USART1 */
    HAL_UART_2,        /**< USART2 */
    HAL_UART_3,        /**< USART3 */
    HAL_UART_4,        /**< UART4 */
    HAL_UART_5,        /**< UART5 */
    HAL_UART_MAX
} hal_uart_id_t;

/**
 * @brief UART数据位
 */
typedef enum {
    HAL_UART_DATA_8 = 0,
    HAL_UART_DATA_9
} hal_uart_databits_t;

/**
 * @brief UART停止位
 */
typedef enum {
    HAL_UART_STOP_1 = 0,
    HAL_UART_STOP_0_5,
    HAL_UART_STOP_2,
    HAL_UART_STOP_1_5
} hal_uart_stopbits_t;

/**
 * @brief UART校验位
 */
typedef enum {
    HAL_UART_PARITY_NONE = 0,
    HAL_UART_PARITY_EVEN,
    HAL_UART_PARITY_ODD
} hal_uart_parity_t;

/**
 * @brief UART配置结构体
 */
typedef struct {
    hal_uart_id_t      id;         /**< UART端口 */
    uint32_t           baudrate;   /**< 波特率 */
    hal_uart_databits_t databits;  /**< 数据位 */
    hal_uart_stopbits_t stopbits;  /**< 停止位 */
    hal_uart_parity_t  parity;     /**< 校验位 */
} hal_uart_config_t;

/**
 * @brief 初始化UART
 * @param config UART配置
 * @return 0成功，其他失败
 */
int hal_uart_init(const hal_uart_config_t *config);

/**
 * @brief 去初始化UART
 * @param id UART端口
 * @return 0成功，其他失败
 */
int hal_uart_deinit(hal_uart_id_t id);

/**
 * @brief 发送数据
 * @param id UART端口
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int hal_uart_send(hal_uart_id_t id, const uint8_t *data, size_t len);

/**
 * @brief 接收数据
 * @param id UART端口
 * @param data 数据指针
 * @param len 数据长度
 * @param timeout_ms 超时时间（毫秒）
 * @return 实际接收的字节数
 */
int hal_uart_recv(hal_uart_id_t id, uint8_t *data, size_t len, uint32_t timeout_ms);

/**
 * @brief 发送字符串
 * @param id UART端口
 * @param str 字符串
 * @return 实际发送的字节数
 */
int hal_uart_send_string(hal_uart_id_t id, const char *str);

/**
 * @brief 获取接收缓冲区中的数据长度
 * @param id UART端口
 * @return 数据长度
 */
int hal_uart_available(hal_uart_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */
