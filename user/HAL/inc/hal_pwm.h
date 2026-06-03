/**
 * @file hal_pwm.h
 * @brief PWM硬件抽象层接口定义
 */

#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief PWM通道定义
 */
typedef enum {
    HAL_PWM_CH1 = 0,
    HAL_PWM_CH2,
    HAL_PWM_CH3,
    HAL_PWM_CH4,
    HAL_PWM_CH_MAX
} hal_pwm_channel_t;

/**
 * @brief PWM配置结构体
 */
typedef struct {
    uint32_t timer_id;
    hal_pwm_channel_t channel;
    uint32_t frequency_hz;
    uint8_t duty_cycle;  /* 0-100 */
} hal_pwm_config_t;

int hal_pwm_init(const hal_pwm_config_t *config);
int hal_pwm_deinit(uint32_t timer_id, hal_pwm_channel_t channel);
int hal_pwm_start(uint32_t timer_id, hal_pwm_channel_t channel);
int hal_pwm_stop(uint32_t timer_id, hal_pwm_channel_t channel);
int hal_pwm_set_duty(uint32_t timer_id, hal_pwm_channel_t channel, uint8_t duty);
int hal_pwm_set_frequency(uint32_t timer_id, hal_pwm_channel_t channel, uint32_t freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* HAL_PWM_H */
