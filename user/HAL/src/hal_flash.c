/**
 * @file hal_flash.c
 * @brief Flash硬件抽象层CH32V30x实现
 */

#include "hal_flash.h"
#include "ch32v30x.h"
#include <stddef.h>

int hal_flash_erase(uint32_t addr, size_t size)
{
    /* 简化实现：按页擦除 */
    (void)size;

    FLASH_Unlock();
    FLASH_ErasePage(addr);
    FLASH_Lock();

    return 0;
}

int hal_flash_write(uint32_t addr, const uint8_t *data, size_t len)
{
    if (data == NULL)
    {
        return -1;
    }

    FLASH_Unlock();

    for (size_t i = 0; i < len; i += 2)
    {
        uint16_t halfword = data[i];
        if (i + 1 < len)
        {
            halfword |= (uint16_t)data[i + 1] << 8;
        }
        FLASH_ProgramHalfWord(addr + i, halfword);
    }

    FLASH_Lock();

    return (int)len;
}

int hal_flash_read(uint32_t addr, uint8_t *data, size_t len)
{
    if (data == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < len; i++)
    {
        data[i] = *(volatile uint8_t *)(addr + i);
    }

    return (int)len;
}

uint32_t hal_flash_get_size(void)
{
    return 256 * 1024;  /* 256KB */
}

uint32_t hal_flash_get_page_size(void)
{
    return 1024;  /* 1KB */
}
