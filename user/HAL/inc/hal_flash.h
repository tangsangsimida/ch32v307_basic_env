/**
 * @file hal_flash.h
 * @brief Flash硬件抽象层接口定义
 */

#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int hal_flash_erase(uint32_t addr, size_t size);
int hal_flash_write(uint32_t addr, const uint8_t *data, size_t len);
int hal_flash_read(uint32_t addr, uint8_t *data, size_t len);
uint32_t hal_flash_get_size(void);
uint32_t hal_flash_get_page_size(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_FLASH_H */
