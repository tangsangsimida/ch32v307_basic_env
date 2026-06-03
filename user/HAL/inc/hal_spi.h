/**
 * @file hal_spi.h
 * @brief SPI硬件抽象层接口定义
 */

#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI端口定义
 */
typedef enum {
    HAL_SPI_1 = 0,
    HAL_SPI_2,
    HAL_SPI_MAX
} hal_spi_id_t;

/**
 * @brief SPI模式
 */
typedef enum {
    HAL_SPI_MODE_MASTER = 0,
    HAL_SPI_MODE_SLAVE
} hal_spi_mode_t;

/**
 * @brief SPI时钟极性
 */
typedef enum {
    HAL_SPI_CPOL_LOW = 0,
    HAL_SPI_CPOL_HIGH
} hal_spi_cpol_t;

/**
 * @brief SPI时钟相位
 */
typedef enum {
    HAL_SPI_CPHA_1EDGE = 0,
    HAL_SPI_CPHA_2EDGE
} hal_spi_cpha_t;

/**
 * @brief SPI数据位
 */
typedef enum {
    HAL_SPI_DATASIZE_8 = 0,
    HAL_SPI_DATASIZE_16
} hal_spi_datasize_t;

/**
 * @brief SPI配置结构体
 */
typedef struct {
    hal_spi_id_t     id;
    hal_spi_mode_t   mode;
    hal_spi_cpol_t   cpol;
    hal_spi_cpha_t   cpha;
    hal_spi_datasize_t datasize;
    uint32_t         clock_hz;
} hal_spi_config_t;

int hal_spi_init(const hal_spi_config_t *config);
int hal_spi_deinit(hal_spi_id_t id);
int hal_spi_transmit(hal_spi_id_t id, const uint8_t *data, size_t len, uint32_t timeout_ms);
int hal_spi_receive(hal_spi_id_t id, uint8_t *data, size_t len, uint32_t timeout_ms);
int hal_spi_transmit_receive(hal_spi_id_t id, const uint8_t *tx_data, uint8_t *rx_data, size_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SPI_H */
