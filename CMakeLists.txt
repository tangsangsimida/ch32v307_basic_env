# ============================================================================
# [移植说明] 更换MCU时需要修改的内容:
#   1. CMAKE_PROJECT_NAME        - 项目名称
#   2. toolchain-riscv.cmake     - TOOLCHAIN_PREFIX / MCU_FLAGS
#   3. Ld/Link.ld                - 链接脚本(内存区域大小和起始地址)
#   4. lib/CMakeLists.txt        - Startup 文件选择(D8 vs D8C)
#   5. add_compile_definitions   - CH32V30x_D8C → CH32V30x_D8
# ============================================================================

cmake_minimum_required(VERSION 3.20)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Toolchain prefix
set(TOOLCHAIN_PREFIX "riscv-none-embed-")

# Project name
project(ch32v307_bare_metal_template C ASM)

# CH32V307 chip definition
add_compile_definitions(CH32V30x_D8C)

# Common compile flags
add_compile_options(
    -Wall
    -fdata-sections
    -ffunction-sections
    -specs=nano.specs
)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Og -g -gdwarf-2)
else()
    add_compile_options(-Os)
endif()

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Ld/Link.ld)

# Find toolchain tools (for post-build commands)
find_program(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
find_program(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(user)

# Create executable
add_executable(${PROJECT_NAME}.elf
    $<TARGET_OBJECTS:ch32v30x_peripheral>
    $<TARGET_OBJECTS:ch32v30x_startup>
    $<TARGET_OBJECTS:user_objects>
)

# Linker flags
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
)

# Link libraries (CMake way)
target_link_libraries(${PROJECT_NAME}.elf PRIVATE c m nosys)

# Post-build: generate HEX and BIN files
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Generating HEX and BIN files..."
)

# Additional clean files
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.bin;${PROJECT_NAME}.map"
)

# Flash target
add_custom_target(flash
    COMMAND wch-isp flash -a 0x00000000 ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing to CH32V307..."
)
