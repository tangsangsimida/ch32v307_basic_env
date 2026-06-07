/**
 * @file eth_led.h
 * @brief 以太网状态 LED 驱动接口
 *
 * 硬件连接（低电平点亮）：
 *   ELED1 - PC10 - Link 状态（亮=Link Up）
 *   ELED2 - PC11 - Activity 活动（有收发帧时短暂亮起）
 */

#ifndef ETH_LED_H
#define ETH_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化以太网 LED GPIO
 *
 * 配置 PC10/PC11 为推挽输出，初始高电平（LED 灭）。
 */
void eth_led_init(void);

/**
 * @brief 更新 Link LED 状态
 * @param link_up 1=Link Up（点亮），0=Link Down（熄灭）
 */
void eth_led_link_update(int link_up);

/**
 * @brief 触发 Activity LED
 *
 * 调用后 LED 点亮，由 eth_led_tick() 在超时后熄灭。
 * 在每次成功发送或接收帧时调用。
 */
void eth_led_activity_trigger(void);

/**
 * @brief Activity LED 超时处理
 *
 * 主循环中定期调用（建议与主循环延时一致），负责在超时后熄灭 Activity LED。
 */
void eth_led_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* ETH_LED_H */
