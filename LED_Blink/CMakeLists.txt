cmake_minimum_required(VERSION 3.20)

# Toolchain prefix
set(TOOLCHAIN_PREFIX "riscv-none-embed-")

# Project name
project(LED_Blink C ASM)

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

# User application sources
set(USER_SOURCES
    User/main.c
    User/ch32v30x_it.c
    User/system_ch32v30x.c
)

# CH32V30x Standard Peripheral Library sources
set(LIB_SOURCES
    LIB/Peripheral/src/ch32v30x_gpio.c
    LIB/Peripheral/src/ch32v30x_rcc.c
    LIB/Peripheral/src/ch32v30x_usart.c
    LIB/Peripheral/src/ch32v30x_misc.c
    LIB/Peripheral/src/ch32v30x_dbgmcu.c
    LIB/Debug/debug.c
)

# Startup assembly
set(ASM_SOURCES
    LIB/Startup/startup_ch32v30x_D8C.S
)

# Include directories
set(INCLUDE_DIRS
    User
    LIB/Peripheral/inc
    LIB/Core
    LIB/Debug
)

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Ld/Link.ld)

# Create executable
add_executable(${PROJECT_NAME}.elf
    ${USER_SOURCES}
    ${LIB_SOURCES}
    ${ASM_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME}.elf PRIVATE ${INCLUDE_DIRS})

# Linker flags
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    -lc -lm -lnosys
)

# Find toolchain tools
find_program(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
find_program(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

# Post-build: generate HEX and BIN files
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Generating HEX and BIN files..."
)

# Flash target
add_custom_target(flash
    COMMAND wch-isp flash -a 0x00000000 ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing to CH32V307..."
)
