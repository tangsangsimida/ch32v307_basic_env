# CH32V30x Standard Peripheral Library

# Auto-collect peripheral sources (CONFIGURE_DEPENDS: auto-detect new files)
file(GLOB PERIPH_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/src/*.c)

# Debug sources
set(DEBUG_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Debug/debug.c
)

# Startup assembly
set(STARTUP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Startup/startup_ch32v30x_D8C.S
)

# Peripheral + debug OBJECT library
add_library(ch32v30x_peripheral OBJECT
    ${PERIPH_SRC}
    ${DEBUG_SRC}
)

# Startup OBJECT library (separate for flexibility)
add_library(ch32v30x_startup OBJECT ${STARTUP_SRC})

# Public include directories (propagated to consumers)
# Note: user/ is included because ch32v30x.h includes
#   system_ch32v30x.h (line 201) and ch32v30x_conf.h (line 6638)
target_include_directories(ch32v30x_peripheral PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Debug
    ${CMAKE_SOURCE_DIR}/user
)
