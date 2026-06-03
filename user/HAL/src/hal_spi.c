/**
 * @file hal_spi.c
 * @brief SPI硬件抽象层CH32V30x实现
 */

#include "hal_spi.h"
#include "hal_system.h"
#include "ch32v30x.h"
#include <stddef.h>

static SPI_TypeDef * const spi_port_map[] = { SPI1, SPI2 };

/**
 * @brief 根据时钟频率计算预分频值
 */
static uint16_t spi_calc_prescaler(uint32_t clock_hz)
{
    uint32_t pclk = SystemCoreClock;

    if (clock_hz >= pclk / 2)
        return SPI_BaudRatePrescaler_2;
    if (clock_hz >= pclk / 4)
        return SPI_BaudRatePrescaler_4;
    if (clock_hz >= pclk / 8)
        return SPI_BaudRatePrescaler_8;
    if (clock_hz >= pclk / 16)
        return SPI_BaudRatePrescaler_16;
    if (clock_hz >= pclk / 32)
        return SPI_BaudRatePrescaler_32;
    if (clock_hz >= pclk / 64)
        return SPI_BaudRatePrescaler_64;
    if (clock_hz >= pclk / 128)
        return SPI_BaudRatePrescaler_128;

    return SPI_BaudRatePrescaler_256;
}

int hal_spi_init(const hal_spi_config_t *config)
{
    SPI_InitTypeDef spi_init;

    if (config == NULL || config->id >= HAL_SPI_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    if (config->id == HAL_SPI_1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    }

    /* 配置SPI */
    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode = (config->mode == HAL_SPI_MODE_MASTER) ? SPI_Mode_Master : SPI_Mode_Slave;
    spi_init.SPI_DataSize = (config->datasize == HAL_SPI_DATASIZE_16) ? SPI_DataSize_16b : SPI_DataSize_8b;
    spi_init.SPI_CPOL = (config->cpol == HAL_SPI_CPOL_HIGH) ? SPI_CPOL_High : SPI_CPOL_Low;
    spi_init.SPI_CPHA = (config->cpha == HAL_SPI_CPHA_2EDGE) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    spi_init.SPI_NSS = SPI_NSS_Soft;
    spi_init.SPI_BaudRatePrescaler = spi_calc_prescaler(config->clock_hz);
    spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial = 7;

    SPI_Init(spi_port_map[config->id], &spi_init);
    SPI_Cmd(spi_port_map[config->id], ENABLE);

    return 0;
}

int hal_spi_deinit(hal_spi_id_t id)
{
    if (id >= HAL_SPI_MAX)
    {
        return -1;
    }

    SPI_Cmd(spi_port_map[id], DISABLE);
    return 0;
}

int hal_spi_transmit(hal_spi_id_t id, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (id >= HAL_SPI_MAX || data == NULL)
    {
        return -1;
    }

    uint32_t start_tick = hal_system_get_tick();

    for (size_t i = 0; i < len; i++)
    {
        /* 等待发送缓冲区空 */
        while (SPI_I2S_GetFlagStatus(spi_port_map[id], SPI_I2S_FLAG_TXE) == RESET)
        {
            if ((hal_system_get_tick() - start_tick) >= timeout_ms)
            {
                return (int)i;
            }
        }

        SPI_I2S_SendData(spi_port_map[id], data[i]);

        /* 等待接收完成 */
        while (SPI_I2S_GetFlagStatus(spi_port_map[id], SPI_I2S_FLAG_RXNE) == RESET)
        {
            if ((hal_system_get_tick() - start_tick) >= timeout_ms)
            {
                return (int)i;
            }
        }

        /* 读取数据清空缓冲区 */
        SPI_I2S_ReceiveData(spi_port_map[id]);
    }

    return (int)len;
}

int hal_spi_receive(hal_spi_id_t id, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (id >= HAL_SPI_MAX || data == NULL)
    {
        return -1;
    }

    uint32_t start_tick = hal_system_get_tick();

    for (size_t i = 0; i < len; i++)
    {
        /* 等待发送缓冲区空 */
        while (SPI_I2S_GetFlagStatus(spi_port_map[id], SPI_I2S_FLAG_TXE) == RESET)
        {
            if ((hal_system_get_tick() - start_tick) >= timeout_ms)
            {
                return (int)i;
            }
        }

        SPI_I2S_SendData(spi_port_map[id], 0xFF);

        /* 等待接收完成 */
        while (SPI_I2S_GetFlagStatus(spi_port_map[id], SPI_I2S_FLAG_RXNE) == RESET)
        {
            if ((hal_system_get_tick() - start_tick) >= timeout_ms)
            {
                return (int)i;
            }
        }

        data[i] = (uint8_t)SPI_I2S_ReceiveData(spi_port_map[id]);
    }

    return (int)len;
}

int hal_spi_transmit_receive(hal_spi_id_t id, const uint8_t *tx_data, uint8_t *rx_data, size_t len, uint32_t timeout_ms)
{
    if (id >= HAL_SPI_MAX || tx_data == NULL || rx_data == NULL)
    {
        return -1;
    }

    uint32_t start_tick = hal_system_get_tick();

    for (size_t i = 0; i < len; i++)
    {
        /* 等待发送缓冲区空 */
        while (SPI_I2S_GetFlagStatus(spi_port_map[id], SPI_I2S_FLAG_TXE) == RESET)
        {
            if ((hal_system_get_tick() - start_tick) >= timeout_ms)
            {
                return (int)i;
            }
        }

        SPI_I2S_SendData(spi_port_map[id], tx_data[i]);

        /* 等待接收完成 */
        while (SPI_I2S_GetFlagStatus(spi_port_map[id], SPI_I2S_FLAG_RXNE) == RESET)
        {
            if ((hal_system_get_tick() - start_tick) >= timeout_ms)
            {
                return (int)i;
            }
        }

        rx_data[i] = (uint8_t)SPI_I2S_ReceiveData(spi_port_map[id]);
    }

    return (int)len;
}
