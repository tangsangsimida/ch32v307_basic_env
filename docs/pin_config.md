# CH32V307 引脚配置表

> 记录项目中所有已使用的引脚分配，避免冲突，方便后续开发查阅。

## 当前已使用引脚

| 引脚 | 网络 | 外设功能 | 代码位置 | 备注 |
|------|------|----------|----------|------|
| PD0 | OSC_IN | HSE 8MHz | system_ch32v30x.c | 时钟输入 |
| PD1 | OSC_OUT | HSE 8MHz | system_ch32v30x.c | 时钟输出 |
| PC14 | OSC32_IN | LSE 32.768kHz | - | RTC 时钟 |
| PC15 | OSC32_OUT | LSE 32.768kHz | - | RTC 时钟 |
| PA9 | TX1 | USART1_TX | debug.c | printf 输出，115200 |
| PA10 | RX1 | USART1_RX | debug.c | printf 接收 |
| PA13 | DIO | SWDIO | - | SWD 调试 |
| PA14 | CLK | SWCLK | - | SWD 调试 |
| PE8 | GPIO | LED1 | user/Peripherals/src/led.c | 推挽输出 |
| PE9 | GPIO | LED2 | user/Peripherals/src/led.c | 推挽输出 |
| PC10 | GPIO | ELED1 (ETH Link) | user/Peripherals/src/eth_led.c | 推挽输出，低电平点亮 |
| PC11 | GPIO | ELED2 (ETH Activity) | user/Peripherals/src/eth_led.c | 推挽输出，低电平点亮 |

### 以太网 RMII 引脚（eth_demo.c）

| 引脚 | 信号 | 方向 | 备注 |
|------|------|------|------|
| PA1 | REFCLK | 输入 | RMII 参考时钟 |
| PA2 | MDIO | 输出 | PHY 管理数据 |
| PA7 | CRSDV | 输入 | 载波侦听/数据有效 |
| PB11 | TXEN | 输出 | 发送使能 |
| PB12 | TXD0 | 输出 | 发送数据 0 |
| PB13 | TXD1 | 输出 | 发送数据 1 |
| PC1 | MDC | 输出 | PHY 管理时钟 |
| PC4 | RXD0 | 输入 | 接收数据 0 |
| PC5 | RXD1 | 输入 | 接收数据 1 |

### USB FS 引脚（usb_cdc_demo.c）

| 引脚 | 信号 | 备注 |
|------|------|------|
| PA11 | USB1_DM | Type-C P7 |
| PA12 | USB1_DP | Type-C P7 |

## 可用 GPIO 引脚

> 以下引脚未被占用，可直接配置为 GPIO 或外设功能。

| 端口 | 可用引脚 |
|------|----------|
| PA | PA0, PA3-PA6, PA8 |
| PB | PB0-PB5, PB6(PB7), PB8-PB10, PB14-PB15 |
| PC | PC0, PC2-PC3, PC6-PC9, PC12-PC13 |
| PD | PD2-PD15 |
| PE | PE0-PE7, PE10-PE15 |

## 注意事项

- PA13/PA14 为 SWD 调试引脚，配置为 GPIO 前需确认不需要在线调试
- PD0/PD1 为 HSE 晶振引脚，不可复用
- PC14/PC15 为 LSE 晶振引脚，使用 RTC 时不可复用
- USART1 默认用于 printf 输出（debug.c 中 `DEBUG` 宏控制）
- 以太网 RMII 占用 PA1/PA2/PA7/PB11-13/PC1/PC4-PC5，这些引脚不可同时用于其他功能
- USB FS 占用 PA11/PA12，与 USB1 Type-C P7 连接器直连
