# CH32V307 裸机开发模板

基于WCH CH32V307 RISC-V微控制器的裸机开发模板，使用CMake构建系统。

## 特性

- **CMake构建系统**：模块化设计，自动收集源文件
- **clangd支持**：自动生成`compile_commands.json`
- **模块化架构**：Application / BSP / Peripherals 三层分离
- **零配置添加文件**：新增`.c/.h`文件无需修改CMakeLists.txt
- **移植友好**：更换MCU时修改点已文档化

## 项目结构

```
ch32v307_bare_metal_template/
├── CMakeLists.txt              # 顶层构建脚本
├── toolchain-riscv.cmake       # RISC-V工具链配置
├── Ld/Link.ld                  # 链接脚本
├── lib/                        # CH32V30x标准外设库
│   ├── CMakeLists.txt          # 外设库构建脚本
│   ├── Peripheral/
│   │   ├── inc/                # 外设头文件
│   │   └── src/                # 外设源文件
│   ├── Core/                   # RISC-V核心文件
│   ├── Debug/                  # 调试和延时函数
│   └── Startup/                # 启动汇编文件
├── user/                       # 用户应用代码
│   ├── CMakeLists.txt          # 自动收集源文件和头文件
│   ├── Application/            # 应用层（业务逻辑）
│   │   ├── inc/
│   │   └── src/main.c
│   ├── BSP/                    # 板级支持包
│   │   ├── inc/
│   │   └── src/
│   └── Peripherals/            # 驱动层
│       ├── inc/
│       └── src/
└── README.md
```

## 开发环境

- **工具链**：riscv-none-embed-gcc (MounRiver Studio)
- **IDE**：VS Code + clangd / MounRiver Studio
- **调试器**：WCH-Link

## 快速开始

### 1. 设置工具链

```bash
# 添加工具链到PATH
export PATH="/path/to/riscv-none-elf/bin:$PATH"
```

### 2. 构建项目

```bash
# 配置（Release模式）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Release

# 配置（Debug模式）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build
```

### 3. 烧录

```bash
# 连接WCH-Link后执行
cmake --build build --target flash
```

### 4. 清理

```bash
rm -rf build
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

## 移植说明

更换MCU时需要修改：

| 文件 | 修改内容 |
|------|----------|
| `CMakeLists.txt` | 项目名称 |
| `toolchain-riscv.cmake` | TOOLCHAIN_PREFIX / MCU_FLAGS |
| `Ld/Link.ld` | 内存区域大小和起始地址 |
| `lib/CMakeLists.txt` | Startup文件选择(D8 vs D8C) |
| `CMakeLists.txt` | CH32V30x_D8C → CH32V30x_D8 |

## 目录说明

| 目录 | 用途 |
|------|------|
| `Application/` | 业务逻辑，与硬件无关 |
| `BSP/` | 板级支持包，系统初始化和配置 |
| `Peripherals/` | 硬件驱动，中断处理 |
| `lib/` | WCH标准外设库，不建议修改 |

## 许可证

MIT License
