# ============================================================================
# 用户代码管理
#
# 本文件自动收集user/目录下所有源文件和头文件, 添加/删除文件时无需修改。
#
# 目录结构:
#   user/
#   +-- Application/    (应用层 - 业务逻辑)
#   |   +-- inc/
#   |   +-- src/
#   +-- HAL/            (硬件抽象层 - 芯片无关接口)
#   |   +-- inc/
#   |   +-- src/
#   +-- BSP/            (板级支持包 - 系统初始化和配置)
#   |   +-- inc/
#   |   +-- src/
#   +-- Peripherals/    (驱动层 - 中断处理和硬件驱动)
#       +-- inc/
#       +-- src/
#
# inc/和src/下的子目录会被自动递归收集。
#
# [移植说明] 本文件与MCU无关, 更换MCU时无需修改。
# ============================================================================

# ---------------------------------------------------------------------------
# 自动收集头文件目录
#
# 通过GLOB_RECURSE找到所有.h文件, 然后提取其父目录。
# 这样在任意子目录中添加头文件会自动将其加入头文件搜索路径。
# ---------------------------------------------------------------------------
file(GLOB_RECURSE USER_Include_Dirs_ABS
    "${CMAKE_CURRENT_SOURCE_DIR}/Application/inc/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HAL/inc/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BSP/inc/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/inc/*.h"
)

set(USER_Include_Dirs "")
foreach(header ${USER_Include_Dirs_ABS})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND USER_Include_Dirs ${dir})
endforeach()
list(REMOVE_DUPLICATES USER_Include_Dirs)

# ---------------------------------------------------------------------------
# 自动收集源文件
# ---------------------------------------------------------------------------
file(GLOB_RECURSE USER_Application_Src
    "${CMAKE_CURRENT_SOURCE_DIR}/Application/src/*.c"
)

file(GLOB_RECURSE USER_HAL_Src
    "${CMAKE_CURRENT_SOURCE_DIR}/HAL/src/*.c"
)

file(GLOB_RECURSE USER_BSP_Src
    "${CMAKE_CURRENT_SOURCE_DIR}/BSP/src/*.c"
)

file(GLOB_RECURSE USER_Peripherals_Src
    "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/src/*.c"
)

set(USER_Sources
    ${USER_Application_Src}
    ${USER_HAL_Src}
    ${USER_BSP_Src}
    ${USER_Peripherals_Src}
)

# ---------------------------------------------------------------------------
# 构建目标
#
# user_interface : 接口库, 导出头文件路径
# user_objects   : 对象库, 编译所有用户源文件, 由根CMakeLists.txt链接到主目标
# ---------------------------------------------------------------------------
add_library(user_interface INTERFACE)
target_include_directories(user_interface INTERFACE
    ${USER_Include_Dirs}
)

add_library(user_objects OBJECT)
target_sources(user_objects PRIVATE
    ${USER_Sources}
)

target_link_libraries(user_objects PUBLIC user_interface ch32v30x_peripheral)

# 将变量暴露给父级CMakeLists.txt
set(USER_Sources ${USER_Sources} PARENT_SCOPE)
set(USER_Include_Dirs ${USER_Include_Dirs} PARENT_SCOPE)
