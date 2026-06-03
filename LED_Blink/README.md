# CH32V307 LED Blink Example

基于WCH官方GPIO_Toggle示例修改的LED闪烁程序。

## 硬件配置

- **MCU**: CH32V307 (RISC-V, rv32imac)
- **开发板**: WCH官方评估板
- **LED引脚**: PA0
- **点亮电平**: 低电平有效（GPIO输出LOW时LED亮）

## 项目结构

```
LED_Blink/
├── User/
│   ├── main.c              # 主程序
│   ├── ch32v30x_it.c       # 中断处理
│   ├── ch32v30x_it.h       # 中断头文件
│   ├── ch32v30x_conf.h     # 外设配置
│   ├── system_ch32v30x.c   # 系统初始化
│   └── system_ch32v30x.h   # 系统头文件
├── LIB/
│   ├── Peripheral/
│   │   ├── inc/            # 外设头文件
│   │   └── src/            # 外设源文件（gpio, rcc, usart, misc, dbgmcu）
│   ├── Core/               # RISC-V核心文件
│   ├── Debug/              # 调试和延时函数
│   └── Startup/            # 启动汇编文件
├── Ld/
│   └── Link.ld             # 链接脚本
├── CMakeLists.txt          # CMake构建脚本
├── toolchain-riscv.cmake   # CMake工具链文件
└── README.md               # 本文件
```

## 构建方法

### 方法一：CMake命令行构建

```bash
# 设置工具链路径
export PATH="/home/dennis/software/riscv-none-elf/bin:$PATH"

# 配置（Release模式）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Release

# 配置（Debug模式）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build

# 清理
rm -rf build

# 烧录（需要WCH-Link连接）
cmake --build build --target flash
```

### 方法二：使用MounRiver Studio

1. 打开MounRiver Studio
2. File → Import → Existing Projects into Workspace
3. 选择LED_Blink目录
4. Project → Build Project
5. Run → Debug As → MounRiver Program

## 代码说明

### LED控制函数

```c
void LED_Init(void);     // 初始化LED GPIO
void LED_On(void);       // 点亮LED（输出低电平）
void LED_Off(void);      // 关闭LED（输出高电平）
void LED_Toggle(void);   // 翻转LED状态
```

### 主循环

```c
while(1)
{
    LED_Toggle();        // 翻转LED
    Delay_Ms(500);       // 延时500ms
}
```

## 闪烁频率

默认闪烁周期：500ms（1Hz频率）

修改 `main.c` 中的 `BLINK_DELAY` 宏可以调整闪烁速度：

```c
#define BLINK_DELAY    500  /* ms */
```

## 串口输出

程序启动后会通过UART1（115200波特率）输出：

```
SystemClk:96000000
ChipID:xxxxxxxx
LED Blink TEST
LED: PA0, Active Low
```

## 库文件说明

项目已包含完整的CH32V30x标准外设库，可独立编译使用：

- **LIB/Peripheral/**: GPIO、RCC、USART、MISC、DBGMCU驱动
- **LIB/Core/**: RISC-V核心定义
- **LIB/Debug**: UART调试输出和延时函数
- **LIB/Startup**: 启动汇编代码
