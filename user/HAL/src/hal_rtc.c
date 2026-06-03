/**
 * @file hal_rtc.c
 * @brief RTC硬件抽象层CH32V30x实现
 */

#include "hal_rtc.h"
#include "ch32v30x.h"
#include <stddef.h>

int hal_rtc_init(void)
{
    /* 使能PWR和BKP时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* 使能BKP访问 */
    PWR_BackupAccessCmd(ENABLE);

    /* 使能LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        ;

    /* 选择LSE作为RTC时钟 */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);

    /* 等待RTC寄存器同步 */
    RTC_WaitForSynchro();

    /* 配置RTC */
    RTC_WaitForLastTask();
    RTC_SetPrescaler(32767);
    RTC_WaitForLastTask();

    return 0;
}

int hal_rtc_deinit(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    BKP_DeInit();
    return 0;
}

int hal_rtc_set_time(const hal_rtc_time_t *time)
{
    if (time == NULL)
    {
        return -1;
    }

    /* 简化实现：只设置秒计数器 */
    /* 实际应用中需要转换为时间戳 */
    RTC_WaitForLastTask();
    RTC_SetCounter(time->second);
    RTC_WaitForLastTask();

    return 0;
}

int hal_rtc_get_time(hal_rtc_time_t *time)
{
    if (time == NULL)
    {
        return -1;
    }

    /* 简化实现：只读取秒计数器 */
    time->second = RTC_GetCounter();
    time->minute = 0;
    time->hour = 0;
    time->day = 1;
    time->month = 1;
    time->year = 2026;
    time->weekday = 0;

    return 0;
}

uint32_t hal_rtc_get_timestamp(void)
{
    return RTC_GetCounter();
}

int hal_rtc_set_alarm(uint32_t seconds)
{
    RTC_WaitForLastTask();
    RTC_SetAlarm(RTC_GetCounter() + seconds);
    RTC_WaitForLastTask();

    return 0;
}

int hal_rtc_set_wakeup(uint32_t seconds)
{
    /* 简化实现 */
    (void)seconds;
    return 0;
}
