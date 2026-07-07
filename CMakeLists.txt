cmake_minimum_required(VERSION 3.15)
project(SpaceShooter VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 禁用 benchmark 自带的测试（避免编译太多东西）
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "")

add_subdirectory(third_party/benchmark)

add_subdirectory(benchmark)

add_subdirectory(src)     

option(BUILD_BENCHMARK "Build benchmark" ON)


# 可选：把生成的可执行文件统一放到 bin 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  

add_executable(MiniSearch src/main.cpp)