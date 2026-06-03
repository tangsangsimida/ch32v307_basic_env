# CH32V30x 标准外设库

# 自动收集外设源文件（CONFIGURE_DEPENDS: 自动检测新文件）
file(GLOB PERIPH_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/src/*.c)

# 调试源文件
set(DEBUG_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Debug/debug.c
)

# 启动汇编文件
set(STARTUP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Startup/startup_ch32v30x_D8C.S
)

# 外设 + 调试 OBJECT库
add_library(ch32v30x_peripheral OBJECT
    ${PERIPH_SRC}
    ${DEBUG_SRC}
)

# 启动OBJECT库（独立以便灵活替换）
add_library(ch32v30x_startup OBJECT ${STARTUP_SRC})

# 公共包含目录（传递给依赖者）
# 注意: 包含user/子目录，因为ch32v30x.h包含了
#   system_ch32v30x.h (第201行) 和 ch32v30x_conf.h (第6638行)
target_include_directories(ch32v30x_peripheral PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Debug
    ${CMAKE_SOURCE_DIR}/user/BSP/inc
    ${CMAKE_SOURCE_DIR}/user/Peripherals/inc
    ${CMAKE_SOURCE_DIR}/user/Application/inc
)
