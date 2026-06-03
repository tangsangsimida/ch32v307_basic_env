# ============================================================================
# [移植说明] 更换MCU时需要修改的内容:
#   1. CMAKE_PROJECT_NAME        - 项目名称
#   2. toolchain-riscv.cmake     - TOOLCHAIN_PREFIX / MCU_FLAGS
#   3. Ld/Link.ld                - 链接脚本(内存区域大小和起始地址)
#   4. lib/CMakeLists.txt        - Startup 文件选择(D8 vs D8C)
#   5. add_compile_definitions   - CH32V30x_D8C → CH32V30x_D8
# ============================================================================

cmake_minimum_required(VERSION 3.20)

# C语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# 默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# 导出编译命令（用于clangd）
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# 工具链前缀
set(TOOLCHAIN_PREFIX "riscv-none-embed-")

# 项目名称
project(ch32v307_bare_metal_template C ASM)

# CH32V307芯片定义
add_compile_definitions(CH32V30x_D8C)

# 通用编译选项
add_compile_options(
    -Wall
    -fdata-sections
    -ffunction-sections
    -specs=nano.specs
)

# 优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Og -g -gdwarf-2)
else()
    add_compile_options(-Os)
endif()

# 链接脚本
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Ld/Link.ld)

# 查找工具链工具（用于构建后命令）
find_program(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
find_program(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

# 添加子目录
add_subdirectory(lib)
add_subdirectory(user)

# 创建可执行文件
add_executable(${PROJECT_NAME}.elf
    $<TARGET_OBJECTS:ch32v30x_peripheral>
    $<TARGET_OBJECTS:ch32v30x_startup>
    $<TARGET_OBJECTS:user_objects>
)

# 链接选项
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
)

# 链接库（CMake标准方式）
target_link_libraries(${PROJECT_NAME}.elf PRIVATE c m nosys)

# 构建后：生成HEX和BIN文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "正在生成HEX和BIN文件..."
)

# 额外的清理文件
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.bin;${PROJECT_NAME}.map"
)

# 烧录目标
add_custom_target(flash
    COMMAND wch-isp flash -a 0x00000000 ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "正在烧录到CH32V307..."
)
