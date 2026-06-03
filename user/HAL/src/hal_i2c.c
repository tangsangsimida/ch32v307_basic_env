/**
 * @file hal_i2c.c
 * @brief I2C硬件抽象层CH32V30x实现
 */

#include "hal_i2c.h"
#include "ch32v30x.h"
#include <stddef.h>

static I2C_TypeDef * const i2c_port_map[] = { I2C1, I2C2 };

int hal_i2c_init(const hal_i2c_config_t *config)
{
    I2C_InitTypeDef i2c_init;

    if (config == NULL || config->id >= HAL_I2C_MAX)
    {
        return -1;
    }

    /* 使能时钟 */
    if (config->id == HAL_I2C_1)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    }

    /* 配置I2C */
    i2c_init.I2C_Mode = I2C_Mode_I2C;
    i2c_init.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c_init.I2C_OwnAddress1 = config->own_address;
    i2c_init.I2C_Ack = I2C_Ack_Enable;
    i2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c_init.I2C_ClockSpeed = config->clock_hz;

    I2C_Init(i2c_port_map[config->id], &i2c_init);
    I2C_Cmd(i2c_port_map[config->id], ENABLE);

    return 0;
}

int hal_i2c_deinit(hal_i2c_id_t id)
{
    if (id >= HAL_I2C_MAX)
    {
        return -1;
    }

    I2C_Cmd(i2c_port_map[id], DISABLE);
    return 0;
}

int hal_i2c_master_transmit(hal_i2c_id_t id, uint16_t dev_addr, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (id >= HAL_I2C_MAX || data == NULL)
    {
        return -1;
    }

    I2C_TypeDef *i2c = i2c_port_map[id];

    /* 等待总线空闲 */
    while (I2C_GetFlagStatus(i2c, I2C_FLAG_BUSY))
        ;

    /* 发送起始条件 */
    I2C_GenerateSTART(i2c, ENABLE);
    while (!I2C_CheckEvent(i2c, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    /* 发送设备地址 */
    I2C_Send7bitAddress(i2c, dev_addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;

    /* 发送数据 */
    for (size_t i = 0; i < len; i++)
    {
        I2C_SendData(i2c, data[i]);
        while (!I2C_CheckEvent(i2c, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
            ;
    }

    /* 发送停止条件 */
    I2C_GenerateSTOP(i2c, ENABLE);

    return (int)len;
}

int hal_i2c_master_receive(hal_i2c_id_t id, uint16_t dev_addr, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (id >= HAL_I2C_MAX || data == NULL)
    {
        return -1;
    }

    I2C_TypeDef *i2c = i2c_port_map[id];

    /* 等待总线空闲 */
    while (I2C_GetFlagStatus(i2c, I2C_FLAG_BUSY))
        ;

    /* 发送起始条件 */
    I2C_GenerateSTART(i2c, ENABLE);
    while (!I2C_CheckEvent(i2c, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    /* 发送设备地址 */
    I2C_Send7bitAddress(i2c, dev_addr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(i2c, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        ;

    /* 接收数据 */
    for (size_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            I2C_AcknowledgeConfig(i2c, DISABLE);
            I2C_GenerateSTOP(i2c, ENABLE);
        }
        while (!I2C_CheckEvent(i2c, I2C_EVENT_MASTER_BYTE_RECEIVED))
            ;
        data[i] = I2C_ReceiveData(i2c);
    }

    I2C_AcknowledgeConfig(i2c, ENABLE);

    return (int)len;
}

int hal_i2c_mem_write(hal_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    /* 简化实现：先发送内存地址，再发送数据 */
    uint8_t addr_buf[2] = { (uint8_t)(mem_addr >> 8), (uint8_t)(mem_addr & 0xFF) };
    (void)hal_i2c_master_transmit(id, dev_addr, addr_buf, 2, timeout_ms);
    return hal_i2c_master_transmit(id, dev_addr, data, len, timeout_ms);
}

int hal_i2c_mem_read(hal_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    /* 简化实现：先发送内存地址，再接收数据 */
    uint8_t addr_buf[2] = { (uint8_t)(mem_addr >> 8), (uint8_t)(mem_addr & 0xFF) };
    (void)hal_i2c_master_transmit(id, dev_addr, addr_buf, 2, timeout_ms);
    return hal_i2c_master_receive(id, dev_addr, data, len, timeout_ms);
}
