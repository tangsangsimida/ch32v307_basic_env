# CH32V307 裸机开发模板

基于WCH CH32V307 RISC-V微控制器的裸机开发模板，使用CMake构建系统和HAL硬件抽象层。

## 特性

- **CMake构建系统**：模块化设计，自动收集源文件
- **clangd支持**：自动生成`compile_commands.json`
- **HAL硬件抽象层**：与芯片无关的统一接口，便于移植
- **模块化架构**：Application / HAL / BSP / Peripherals 四层分离
- **零配置添加文件**：新增`.c/.h`文件无需修改CMakeLists.txt
- **移植友好**：更换芯片时只需重写HAL实现层

## 项目结构

```
ch32v307_bare_metal_template/
├── CMakeLists.txt              # 顶层构建脚本
├── env.cmake                   # 环境配置（用户修改此文件）
├── toolchain-riscv.cmake       # RISC-V工具链配置
├── tools/                      # 工具脚本
│   ├── setup_vscode.sh         # Linux/macOS配置脚本
│   └── setup_vscode.bat        # Windows配置脚本
├── Ld/Link.ld                  # 链接脚本
├── lib/                        # CH32V30x标准外设库
│   ├── CMakeLists.txt          # 外设库构建脚本
│   ├── Peripheral/             # 外设驱动
│   ├── Core/                   # RISC-V核心
│   ├── Debug/                  # 调试支持
│   └── Startup/                # 启动代码
├── user/                       # 用户代码
│   ├── CMakeLists.txt          # 自动收集源文件
│   ├── Application/            # 应用层（业务逻辑）
│   │   ├── inc/
│   │   └── src/main.c
│   ├── HAL/                    # 硬件抽象层
│   │   ├── inc/                # HAL接口定义
│   │   └── src/                # HAL实现（CH32V30x）
│   ├── BSP/                    # 板级支持包
│   │   ├── inc/
│   │   └── src/
│   └── Peripherals/            # 中断处理
│       ├── inc/
│       └── src/
└── README.md
```

## 环境配置检查清单

使用本模板前，请确认以下配置项：

### 必须配置

| 配置项 | 文件 | 说明 |
|--------|------|------|
| **工具链路径** | `env.cmake` | 修改 `TOOLCHAIN_PATH` 为你的工具链安装路径 |
| **链接脚本** | `Ld/Link.ld` | 根据实际芯片型号调整FLASH/RAM大小 |
| **芯片型号** | `CMakeLists.txt` | 确认 `CH32V30x_D8C` 或 `CH32V30x_D8` |

### 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| `riscv-none-embed-gcc: not found` | 工具链路径错误 | 检查 `env.cmake` 中的 `TOOLCHAIN_PATH` |
| `cannot find -lc` | 缺少newlib库 | 确保工具链包含newlib（MounRiver Studio自带） |
| VS Code无法识别头文件 | compile_commands.json不存在 | 先运行 `cmake -B build` 生成 |

## 快速开始

### 1. 配置环境

编辑 `env.cmake` 文件，设置工具链路径：

```cmake
# 修改为你的工具链路径
set(TOOLCHAIN_PATH "/home/dennis/software/riscv-none-elf/bin")
```

### 2. 配置VS Code（可选）

**Linux/macOS:**
```bash
./tools/setup_vscode.sh
```

**Windows:**
```cmd
tools\setup_vscode.bat
```

### 3. 构建项目

```bash
# 配置
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build

# 清理
rm -rf build
```

### 4. 烧录

```bash
cmake --build build --target flash
```

## HAL硬件抽象层

### 支持的外设

| 接口 | 头文件 | 功能 |
|------|--------|------|
| GPIO | hal_gpio.h | 通用输入输出 |
| UART | hal_uart.h | 串口通信 |
| SPI | hal_spi.h | SPI通信 |
| I2C | hal_i2c.h | I2C通信 |
| ADC | hal_adc.h | 模数转换 |
| PWM | hal_pwm.h | 脉宽输出 |
| Timer | hal_timer.h | 定时器 |
| RTC | hal_rtc.h | 实时时钟 |
| WDG | hal_wdg.h | 看门狗 |
| DMA | hal_dma.h | 直接内存访问 |
| Flash | hal_flash.h | Flash读写 |
| CAN | hal_can.h | CAN总线 |
| System | hal_system.h | 系统功能 |

### 使用示例

```c
#include "hal.h"

int main(void)
{
    /* 初始化HAL */
    hal_init();

    /* 配置GPIO */
    hal_gpio_config_t gpio_config = {
        .port = HAL_GPIO_PORT_A,
        .pin = HAL_GPIO_PIN_0,
        .mode = HAL_GPIO_MODE_OUTPUT_PP,
        .speed = HAL_GPIO_SPEED_HIGH
    };
    hal_gpio_init(&gpio_config);

    /* 主循环 */
    while (1)
    {
        hal_gpio_toggle(HAL_GPIO_PORT_A, HAL_GPIO_PIN_0);
        hal_delay_ms(500);
    }
}
```

### 移植指南

更换芯片时：

1. **保留接口定义**：`user/HAL/inc/` 中的头文件不变
2. **重写实现**：修改 `user/HAL/src/` 中的 `.c` 文件
3. **业务代码不变**：`user/Application/` 中的代码无需修改

```
更换芯片前：
  Application → HAL接口 → CH32实现

更换芯片后：
  Application → HAL接口 → 新芯片实现
```

## 添加新文件

### 添加驱动文件
```
user/Peripherals/inc/led.h
user/Peripherals/src/led.c
```

### 添加业务逻辑
```
user/Application/src/app.c
user/Application/inc/app.h
```

**无需修改CMakeLists.txt**，直接构建即可。

## VS Code快捷键

| 快捷键 | 功能 |
|--------|------|
| `F7` | 构建项目 |
| `Ctrl+Shift+B` | 构建项目 |
| `Ctrl+Shift+C` | 清理构建 |
| `Ctrl+Shift+R` | 重新构建 |
| `Ctrl+Shift+F` | 烧录到MCU |

## 移植说明

更换MCU时需要修改：

| 文件 | 修改内容 |
|------|----------|
| `env.cmake` | TOOLCHAIN_PATH（如果工具链不同） |
| `CMakeLists.txt` | 项目名称、芯片型号 |
| `Ld/Link.ld` | 内存区域大小和起始地址 |
| `lib/CMakeLists.txt` | Startup文件选择 |
| `user/HAL/src/` | 重写HAL实现 |

## 目录说明

| 目录 | 用途 |
|------|------|
| `Application/` | 业务逻辑，与硬件无关 |
| `HAL/` | 硬件抽象层，芯片无关接口 |
| `BSP/` | 板级支持包，系统初始化 |
| `Peripherals/` | 中断处理 |
| `lib/` | WCH标准外设库 |
| `tools/` | 工具脚本 |

## 工具链安装

从以下地址下载MounRiver Studio：
- 官网：http://www.wch-ic.com/downloads/MounRiver_STUDIO_IDE_ZIP.html

安装后，`env.cmake` 中的路径设置为：

| 操作系统 | 路径示例 |
|----------|----------|
| Windows | `C:/MounRiver/Toolchain/RISC-V Embedded GCC/bin` |
| Linux | `/home/user/MounRiver/Toolchain/RISC-V Embedded GCC/bin` |
| macOS | `/Applications/MounRiver/Toolchain/RISC-V Embedded GCC/bin` |

## 许可证

MIT License
