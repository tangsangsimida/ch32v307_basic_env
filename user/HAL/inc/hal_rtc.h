/**
 * @file hal_rtc.h
 * @brief RTC硬件抽象层接口定义
 */

#ifndef HAL_RTC_H
#define HAL_RTC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 时间结构体
 */
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  weekday;  /* 0=周日, 1=周一, ... */
} hal_rtc_time_t;

int hal_rtc_init(void);
int hal_rtc_deinit(void);
int hal_rtc_set_time(const hal_rtc_time_t *time);
int hal_rtc_get_time(hal_rtc_time_t *time);
uint32_t hal_rtc_get_timestamp(void);
int hal_rtc_set_alarm(uint32_t seconds);
int hal_rtc_set_wakeup(uint32_t seconds);

#ifdef __cplusplus
}
#endif

#endif /* HAL_RTC_H */
