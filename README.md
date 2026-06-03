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
├── env.cmake                   # 环境配置（用户修改此文件）
├── toolchain-riscv.cmake       # RISC-V工具链配置
├── tools/                      # 工具脚本
│   ├── setup_vscode.sh         # Linux/macOS配置脚本
│   └── setup_vscode.bat        # Windows配置脚本
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

## 环境配置检查清单

使用本模板前，请确认以下配置项，否则可能导致构建失败：

### 必须配置

| 配置项 | 文件 | 说明 |
|--------|------|------|
| **工具链路径** | `env.cmake` | 修改 `TOOLCHAIN_PATH` 为你的工具链安装路径 |
| **链接脚本** | `Ld/Link.ld` | 根据实际芯片型号调整FLASH/RAM大小 |
| **芯片型号** | `CMakeLists.txt` | 确认 `CH32V30x_D8C` 或 `CH32V30x_D8` |

### 可能需要配置

| 配置项 | 文件 | 说明 |
|--------|------|------|
| **烧录工具** | `CMakeLists.txt` | 默认使用 `wch-isp`，如使用其他工具需修改 |
| **clangd路径** | `.vscode/settings.json` | Linux通常为 `/usr/bin/clangd`，Windows需安装LLVM |
| **调试器** | - | 确保WCH-Link驱动已安装 |

### 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| `riscv-none-embed-gcc: not found` | 工具链路径错误 | 检查 `env.cmake` 中的 `TOOLCHAIN_PATH` |
| `cannot find -lc` | 缺少newlib库 | 确保工具链包含newlib（MounRiver Studio自带） |
| `undefined reference to _start` | 启动文件缺失 | 检查 `lib/Startup/startup_ch32v30x_D8C.S` 是否存在 |
| `system_ch32v30x.h: No such file` | 头文件路径错误 | 检查 `user/BSP/inc/` 目录 |
| VS Code无法识别头文件 | compile_commands.json不存在 | 先运行 `cmake -B build` 生成 |

## 快速开始

### 1. 配置环境

编辑 `env.cmake` 文件，设置工具链路径：

```cmake
# 修改为你的工具链路径（注意路径分隔符）
# Windows: "C:/MounRiver/Toolchain/RISC-V Embedded GCC/bin"
# Linux: "/home/user/MounRiver/Toolchain/RISC-V Embedded GCC/bin"
set(TOOLCHAIN_PATH "/home/dennis/software/riscv-none-elf/bin")
```

### 2. 配置VS Code（可选）

运行配置脚本自动生成VS Code配置：

**Linux/macOS:**
```bash
./tools/setup_vscode.sh
```

**Windows:**
```cmd
tools\setup_vscode.bat
```

然后重新加载VS Code。

### 3. 构建项目

#### 命令行方式

```bash
# 配置（Debug模式）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Debug

# 配置（Release模式）
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build
```

#### VS Code方式

- 按 `F7` 构建项目
- 按 `Ctrl+Shift+C` 清理构建
- 按 `Ctrl+Shift+R` 重新构建

### 4. 烧录

```bash
# 命令行
cmake --build build --target flash

# VS Code
按 Ctrl+Shift+F
```

### 5. 清理

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

## 环境配置说明

### env.cmake

```cmake
# 工具链路径（用户需要修改为自己的路径）
set(TOOLCHAIN_PATH "/home/dennis/software/riscv-none-elf/bin")

# 工具链前缀（通常不需要修改）
set(TOOLCHAIN_PREFIX "riscv-none-embed-")
```

### 工具链安装

从以下地址下载MounRiver Studio：
- 官网：http://www.wch-ic.com/downloads/MounRiver_STUDIO_IDE_ZIP.html

安装后，`env.cmake` 中的路径设置为：

| 操作系统 | 路径示例 |
|----------|----------|
| Windows | `C:/MounRiver/Toolchain/RISC-V Embedded GCC/bin` |
| Linux | `/home/user/MounRiver/Toolchain/RISC-V Embedded GCC/bin` |
| macOS | `/Applications/MounRiver/Toolchain/RISC-V Embedded GCC/bin` |

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
| `CMakeLists.txt` | 项目名称 |
| `Ld/Link.ld` | 内存区域大小和起始地址 |
| `lib/CMakeLists.txt` | Startup文件选择(D8 vs D8C) |
| `CMakeLists.txt` | CH32V30x_D8C → CH32V30x_D8 |

## 目录说明

| 目录 | 用途 |
|------|------|
| `tools/` | 工具脚本 |
| `Application/` | 业务逻辑，与硬件无关 |
| `BSP/` | 板级支持包，系统初始化和配置 |
| `Peripherals/` | 硬件驱动，中断处理 |
| `lib/` | WCH标准外设库，不建议修改 |

## 许可证

MIT License
