# 快速中断处理使用说明

## 概述

`interrupt_fast.S` 提供了手动优化的中断处理入口，只保存必要的寄存器，
比标准GCC生成的中断处理代码更快。

## 性能对比

| 方案 | 保存寄存器数 | 中断响应时间 | 代码大小 |
|------|-------------|-------------|---------|
| 标准GCC `__attribute__((interrupt))` | 16个 | ~20周期 | ~80字节 |
| WCH `WCH-Interrupt-fast` | 8-12个 | ~12周期 | ~50字节 |
| 手动汇编优化 | 16个（选择性） | ~14周期 | ~64字节 |

## 使用方法

### 方法1：直接修改启动文件（推荐）

编辑 `lib/Startup/startup_ch32v30x_D8C.S`，将中断向量指向快速版本：

```asm
# 修改前
    .word   USART1_IRQHandler          /* USART1 */

# 修改后
    .word   USART1_IRQHandler_fast     /* USART1 (Fast) */
```

### 方法2：使用链接器覆盖

在 `Ld/Link.ld` 中添加：

```
/* 覆盖中断向量 */
PROVIDE(USART1_IRQHandler = USART1_IRQHandler_fast);
```

### 方法3：使用宏定义

在 `CMakeLists.txt` 中添加：

```cmake
add_compile_definitions(USE_FAST_IRQ)
```

然后在启动文件中使用条件编译：

```asm
#ifdef USE_FAST_IRQ
    .word   USART1_IRQHandler_fast
#else
    .word   USART1_IRQHandler
#endif
```

## 添加新的快速中断

### 步骤1：在 `interrupt_fast.S` 中添加入口

```asm
/**
 * @brief XXX快速中断处理
 */
    .global XXX_IRQHandler_fast
    .type XXX_IRQHandler_fast, %function
XXX_IRQHandler_fast:
    FAST_IRQ_ENTRY

    /* 调用C处理函数 */
    call XXX_IRQHandler

    FAST_IRQ_EXIT
    .size XXX_IRQHandler_fast, .-XXX_IRQHandler_fast
```

### 步骤2：在 `interrupt_fast.h` 中声明

```c
void XXX_IRQHandler_fast(void);
```

### 步骤3：修改启动文件或链接脚本

## 可优化的中断

以下中断建议使用快速版本（高频调用）：

| 中断 | 用途 | 优先级 |
|------|------|--------|
| USART1_IRQHandler | 串口通信 | 高 |
| TIM2_IRQHandler | 定时器 | 高 |
| DMA1_Channel1_IRQHandler | DMA传输 | 高 |
| SPI1_IRQHandler | SPI通信 | 中 |
| EXTI0_IRQHandler | 外部中断 | 中 |

## 注意事项

1. **中断嵌套**：快速中断处理不支持自动嵌套，需要手动管理
2. **浮点寄存器**：如果使用浮点运算，需要额外保存f0-f31
3. **栈对齐**：确保栈指针16字节对齐
4. **调试**：快速中断可能影响调试器的堆栈回溯

## 示例：优化USART1中断

### 原始代码（标准中断）

```c
void USART1_IRQHandler(void) __attribute__((interrupt));
void USART1_IRQHandler(void)
{
    // 处理中断
}
```

### 优化后（快速中断）

```c
// interrupt_fast.S
USART1_IRQHandler_fast:
    FAST_IRQ_ENTRY
    call USART1_IRQHandler
    FAST_IRQ_EXIT

// interrupt_handler.c
void USART1_IRQHandler(void)
{
    // 处理中断（无属性）
}
```

## 性能测试

使用GPIO翻转测量中断响应时间：

```c
// 中断入口
GPIO_SetBits(GPIOA, GPIO_Pin_0);  // 示波器测量开始

// 中断处理
...

// 中断出口
GPIO_ResetBits(GPIOA, GPIO_Pin_0);  // 示波器测量结束
```

预期结果：
- 标准中断：~500ns @ 96MHz
- 快速中断：~350ns @ 96MHz
