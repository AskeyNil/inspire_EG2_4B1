# 定义变量
set(TALON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/talon)

# 指定包含头文件的路径
include_directories(
    ${TALON_DIR}
)

# 添加库
add_library(
    Talon
    ${TALON_DIR}/Talon.cc
    ${TALON_DIR}/rs232.c
)


