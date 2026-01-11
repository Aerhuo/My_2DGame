cmake_minimum_required(VERSION 3.10)

# 项目名称
project(My_2DGame)

# 设置 C 标准 (C99 或 C11 都可以)
set(CMAKE_C_STANDARD 11)

# 1. 自动扫描 src 文件夹下所有的 .c 文件
file(GLOB SOURCES "src/*.c")

# 2. 创建可执行文件
add_executable(Game ${SOURCES})

# 3. 自动关联头文件目录
target_include_directories(Game PRIVATE include)