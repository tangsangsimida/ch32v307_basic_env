# ============================================================================
# 环境配置文件（降级方案）
#
# 推荐方式：设置环境变量 RISCV_TOOLCHAIN_PATH，无需修改此文件
#   Linux:   export RISCV_TOOLCHAIN_PATH=/path/to/toolchain/bin
#   Windows: set RISCV_TOOLCHAIN_PATH=C:\path\to\toolchain\bin
#
# 当环境变量未设置时，此文件作为降级方案生效。
# ============================================================================

# 工具链路径（降级配置，仅在环境变量未设置时生效）
set(TOOLCHAIN_PATH "/home/dennis/software/riscv-none-elf/bin")

# 工具链前缀（通常不需要修改）
set(TOOLCHAIN_PREFIX "riscv-none-embed-")
