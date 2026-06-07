# CH32V307 RISC-V 工具链配置文件
#
# 工具链路径来源（优先级从高到低）：
#   1. 环境变量 RISCV_TOOLCHAIN_PATH + RISCV_TOOLCHAIN_PREFIX
#   2. env.cmake 中的 TOOLCHAIN_PATH + TOOLCHAIN_PREFIX
#
# 跨平台使用：设置环境变量即可，无需修改任何文件
#   Linux:   export RISCV_TOOLCHAIN_PATH=/path/to/toolchain/bin
#   Windows: set RISCV_TOOLCHAIN_PATH=C:\path\to\toolchain\bin

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv)

# ============================================================================
# 工具链路径配置：环境变量优先，降级到 env.cmake
# ============================================================================
if(DEFINED ENV{RISCV_TOOLCHAIN_PATH})
    # 使用环境变量（跨平台推荐方式）
    set(TOOLCHAIN_PATH "$ENV{RISCV_TOOLCHAIN_PATH}")
    if(DEFINED ENV{RISCV_TOOLCHAIN_PREFIX})
        set(TOOLCHAIN_PREFIX "$ENV{RISCV_TOOLCHAIN_PREFIX}")
    else()
        set(TOOLCHAIN_PREFIX "riscv-none-embed-")
    endif()
else()
    # 降级：从 env.cmake 读取（命令行构建兼容）
    include(${CMAKE_CURRENT_LIST_DIR}/env.cmake)
endif()

# 验证编译器是否存在
if(NOT EXISTS "${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc")
    message(FATAL_ERROR
        "找不到 RISC-V 编译器: ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc\n"
        "请设置环境变量 RISCV_TOOLCHAIN_PATH 指向工具链 bin 目录，\n"
        "或修改 env.cmake 中的 TOOLCHAIN_PATH。\n"
        "示例: export RISCV_TOOLCHAIN_PATH=/path/to/riscv-none-embed/bin"
    )
endif()

# ============================================================================
# 编译器设置
# ============================================================================
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}ar)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_SIZE ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}size)

# 跳过编译器测试（编译+链接），只测试编译本身
# 裸机工具链没有 crt0.o / libc，链接测试必然失败
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# MCU架构标志
set(MCU_FLAGS "-march=rv32imac_zicsr_zifencei -mabi=ilp32")

set(CMAKE_C_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCU_FLAGS} -nostartfiles -specs=nano.specs")

# 不使用标准库
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
