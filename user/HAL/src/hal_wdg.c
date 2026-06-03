/**
 * @file hal_wdg.c
 * @brief 看门狗硬件抽象层CH32V30x实现
 */

#include "hal_wdg.h"
#include "ch32v30x.h"
#include <stddef.h>

static hal_wdg_type_t current_wdg_type = HAL_WDG_IWDG;

/**
 * @brief 计算IWDG预分频器和重载值
 *
 * IWDG时钟 = LSI / 预分频器
 * 超时时间 = 重载值 / IWDG时钟
 * LSI约40KHz
 */
static void iwdg_calc_params(uint32_t timeout_ms, uint16_t *prescaler, uint16_t *reload)
{
    /* 预分频器选项: 4, 8, 16, 32, 64, 128, 256 */
    uint16_t prescaler_values[] = { 4, 8, 16, 32, 64, 128, 256 };
    uint16_t prescaler_regs[] = { IWDG_Prescaler_4, IWDG_Prescaler_8, IWDG_Prescaler_16,
                                   IWDG_Prescaler_32, IWDG_Prescaler_64, IWDG_Prescaler_128,
                                   IWDG_Prescaler_256 };

    /* LSI时钟频率约40KHz */
    uint32_t lsi_freq = 40000;

    for (int i = 0; i < 7; i++)
    {
        uint32_t iwdg_freq = lsi_freq / prescaler_values[i];
        uint32_t reload_val = (timeout_ms * iwdg_freq) / 1000;

        if (reload_val <= 0xFFF)  /* 12位最大值 */
        {
            *prescaler = prescaler_regs[i];
            *reload = (uint16_t)reload_val;
            return;
        }
    }

    /* 如果超时太长，使用最大预分频器 */
    *prescaler = IWDG_Prescaler_256;
    *reload = 0xFFF;
}

int hal_wdg_init(const hal_wdg_config_t *config)
{
    if (config == NULL)
    {
        return -1;
    }

    current_wdg_type = config->type;

    if (config->type == HAL_WDG_IWDG)
    {
        uint16_t prescaler, reload;
        iwdg_calc_params(config->timeout_ms, &prescaler, &reload);

        /* 使能IWDG */
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

        /* 设置预分频器 */
        IWDG_SetPrescaler(prescaler);

        /* 设置重载值 */
        IWDG_SetReload(reload);

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
