/**
 * @file hal_wdg.c
 * @brief 看门狗硬件抽象层CH32V30x实现
 */

#include "hal_wdg.h"
#include "ch32v30x.h"
#include <stddef.h>

static hal_wdg_type_t current_wdg_type = HAL_WDG_IWDG;

int hal_wdg_init(const hal_wdg_config_t *config)
{
    if (config == NULL)
    {
        return -1;
    }

    current_wdg_type = config->type;

    if (config->type == HAL_WDG_IWDG)
    {
        /* 使能IWDG */
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

        /* 设置预分频器 */
        IWDG_SetPrescaler(IWDG_Prescaler_256);

        /* 设置重载值 */
        IWDG_SetReload(config->timeout_ms * 40 / 256);

        /* 重载计数器 */
        IWDG_ReloadCounter();

        /* 使能IWDG */
        IWDG_Enable();
    }
    else
    {
        /* WWDG配置 */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

        WWDG_SetPrescaler(WWDG_Prescaler_8);
        WWDG_SetWindowValue(0x7F);
        WWDG_Enable(0x7F);
    }

    return 0;
}

int hal_wdg_deinit(void)
{
    /* 看门狗一旦使能无法关闭，只能通过复位 */
    return 0;
}

int hal_wdg_feed(void)
{
    if (current_wdg_type == HAL_WDG_IWDG)
    {
        IWDG_ReloadCounter();
    }
    else
    {
        WWDG_SetCounter(0x7F);
    }

    return 0;
}

int hal_wdg_enable(void)
{
    if (current_wdg_type == HAL_WDG_IWDG)
    {
        IWDG_Enable();
    }
    else
    {
        WWDG_Enable(0x7F);
    }

    return 0;
}

int hal_wdg_disable(void)
{
    /* 看门狗一旦使能无法关闭 */
    return -1;
}
