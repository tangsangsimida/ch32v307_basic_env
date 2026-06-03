# ============================================================================
# 环境配置文件
#
# 用户需要根据自己的环境修改以下配置：
#   1. TOOLCHAIN_PATH - 工具链安装路径
#
# 其他CMake文件通过include此文件获取环境变量。
# ============================================================================

# 工具链路径（用户需要修改为自己的路径）
# 示例: /home/user/MounRiver/Toolchain/RISC-V Embedded GCC/bin
set(TOOLCHAIN_PATH "/home/dennis/software/riscv-none-elf/bin")

# 工具链前缀（通常不需要修改）
set(TOOLCHAIN_PREFIX "riscv-none-embed-")
