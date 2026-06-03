/**
 * @file hal_can.h
 * @brief CAN硬件抽象层接口定义
 */

#ifndef HAL_CAN_H
#define HAL_CAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CAN端口定义
 */
typedef enum {
    HAL_CAN_1 = 0,
    HAL_CAN_MAX
} hal_can_id_t;

/**
 * @brief CAN帧结构体
 */
typedef struct {
    uint32_t id;
    uint8_t  ide;       /* 0=标准帧, 1=扩展帧 */
    uint8_t  rtr;       /* 0=数据帧, 1=远程帧 */
    uint8_t  dlc;       /* 数据长度 0-8 */
    uint8_t  data[8];
} hal_can_msg_t;

/**
 * @brief CAN配置结构体
 */
typedef struct {
    hal_can_id_t id;
    uint32_t baudrate;
    uint32_t mode;      /* 0=正常, 1=回环, 2=静默 */
} hal_can_config_t;

int hal_can_init(const hal_can_config_t *config);
int hal_can_deinit(hal_can_id_t id);
int hal_can_transmit(hal_can_id_t id, const hal_can_msg_t *msg, uint32_t timeout_ms);
int hal_can_receive(hal_can_id_t id, hal_can_msg_t *msg, uint32_t timeout_ms);
int hal_can_set_filter(hal_can_id_t id, uint32_t id_mask, uint32_t mask);
int hal_can_set_callback(hal_can_id_t id, void (*callback)(hal_can_msg_t *msg));

#ifdef __cplusplus
}
#endif

#endif /* HAL_CAN_H */
