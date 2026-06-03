/**
 * @file hal_i2c.h
 * @brief I2C硬件抽象层接口定义
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief I2C端口定义
 */
typedef enum {
    HAL_I2C_1 = 0,
    HAL_I2C_2,
    HAL_I2C_MAX
} hal_i2c_id_t;

/**
 * @brief I2C配置结构体
 */
typedef struct {
    hal_i2c_id_t id;
    uint32_t     clock_hz;
    uint16_t     own_address;
} hal_i2c_config_t;

int hal_i2c_init(const hal_i2c_config_t *config);
int hal_i2c_deinit(hal_i2c_id_t id);
int hal_i2c_master_transmit(hal_i2c_id_t id, uint16_t dev_addr, const uint8_t *data, size_t len, uint32_t timeout_ms);
int hal_i2c_master_receive(hal_i2c_id_t id, uint16_t dev_addr, uint8_t *data, size_t len, uint32_t timeout_ms);
int hal_i2c_mem_write(hal_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, const uint8_t *data, size_t len, uint32_t timeout_ms);
int hal_i2c_mem_read(hal_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, size_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* HAL_I2C_H */
