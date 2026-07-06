cmake_minimum_required(VERSION 3.15)
project(SpaceShooter VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)     

# 可选：把生成的可执行文件统一放到 bin 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(MiniSearch src/main.cpp)