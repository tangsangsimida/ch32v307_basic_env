/**
 * @file hal_flash.c
 * @brief Flash硬件抽象层CH32V30x实现
 */

#include "hal_flash.h"
#include "ch32v30x.h"

/**
 * @brief Flash页大小
 */
#define FLASH_PAGE_SIZE  1024

int hal_flash_erase(uint32_t addr, size_t size)
{
    /* 计算需要擦除的页数 */
    uint32_t pages = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    FLASH_Unlock();

    for (uint32_t i = 0; i < pages; i++)
    {
        FLASH_ErasePage(addr + i * FLASH_PAGE_SIZE);
    }

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
    /* 根据芯片ID判断Flash大小 */
    uint32_t chip_id = DBGMCU_GetCHIPID();

    /* CH32V307: 256KB, CH32V305: 128KB, CH32V303: 64KB */
    if ((chip_id & 0xFFFF) == 0x3070)
    {
        return 256 * 1024;
    }
    else if ((chip_id & 0xFFFF) == 0x3050)
    {
        return 128 * 1024;
    }
    else
    {
        return 64 * 1024;
    }
}

uint32_t hal_flash_get_page_size(void)
{
    return FLASH_PAGE_SIZE;
}
