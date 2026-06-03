/**
 * @file hal_rtc.c
 * @brief RTC硬件抽象层CH32V30x实现
 */

#include "hal_rtc.h"
#include "ch32v30x.h"
#include <stddef.h>

/**
 * @brief 每月天数表（非闰年）
 */
static const uint8_t days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/**
 * @brief 判断是否为闰年
 */
static int is_leap_year(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/**
 * @brief 获取某月天数
 */
static uint8_t get_days_in_month(uint16_t year, uint8_t month)
{
    if (month == 2 && is_leap_year(year))
    {
        return 29;
    }
    return days_in_month[month - 1];
}

/**
 * @brief 时间结构体转时间戳
 */
static uint32_t rtc_time_to_timestamp(const hal_rtc_time_t *time)
{
    uint32_t timestamp = 0;

    /* 计算从1970年到指定年份的秒数 */
    for (uint16_t y = 1970; y < time->year; y++)
    {
        timestamp += is_leap_year(y) ? 366 * 86400 : 365 * 86400;
    }

    /* 计算从1月到指定月份的秒数 */
    for (uint8_t m = 1; m < time->month; m++)
    {
        timestamp += get_days_in_month(time->year, m) * 86400;
    }

    /* 计算天、时、分、秒 */
    timestamp += (time->day - 1) * 86400;
    timestamp += time->hour * 3600;
    timestamp += time->minute * 60;
    timestamp += time->second;

    return timestamp;
}

/**
 * @brief 时间戳转时间结构体
 */
static void rtc_timestamp_to_time(uint32_t timestamp, hal_rtc_time_t *time)
{
    /* 计算年份 */
    uint16_t year = 1970;
    while (1)
    {
        uint32_t seconds_in_year = is_leap_year(year) ? 366 * 86400 : 365 * 86400;
        if (timestamp < seconds_in_year)
        {
            break;
        }
        timestamp -= seconds_in_year;
        year++;
    }
    time->year = year;

    /* 计算月份 */
    uint8_t month = 1;
    while (month <= 12)
    {
        uint32_t seconds_in_month = get_days_in_month(year, month) * 86400;
        if (timestamp < seconds_in_month)
        {
            break;
        }
        timestamp -= seconds_in_month;
        month++;
    }
    time->month = month;

    /* 计算天、时、分、秒 */
    time->day = timestamp / 86400 + 1;
    timestamp %= 86400;
    time->hour = timestamp / 3600;
    timestamp %= 3600;
    time->minute = timestamp / 60;
    time->second = timestamp % 60;

    /* 计算星期（蔡勒公式） */
    int y = time->year;
    int m = time->month;
    if (m < 3)
    {
        m += 12;
        y--;
    }
    int c = y / 100;
    y = y % 100;
    time->weekday = (y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + time->day - 1) % 7;
    if (time->weekday < 0)
    {
        time->weekday += 7;
    }
}

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

    uint32_t timestamp = rtc_time_to_timestamp(time);

    RTC_WaitForLastTask();
    RTC_SetCounter(timestamp);
    RTC_WaitForLastTask();

    return 0;
}

int hal_rtc_get_time(hal_rtc_time_t *time)
{
    if (time == NULL)
    {
        return -1;
    }

    uint32_t timestamp = RTC_GetCounter();
    rtc_timestamp_to_time(timestamp, time);

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
