cmake_minimum_required(VERSION 3.10)

# 项目名称
project(My_2DGame)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)

# 自动链接源文件
file(GLOB SOURCES "src/*.c")

# 创建可执行文件
add_executable(Game ${SOURCES})

# 自动关联头文件目录
target_include_directories(Game PRIVATE include)