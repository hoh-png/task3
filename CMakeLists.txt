cmake_minimum_required(VERSION 3.15)
project(MiniSearch VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 禁用 benchmark 自带的测试（避免编译太多东西）
option(BUILD_BENCHMARK "Build benchmark" ON)

if(BUILD_BENCHMARK)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
    set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "")
    add_subdirectory(third_party/benchmark)
    add_subdirectory(benchmark)
endif()

add_subdirectory(src)


# 可选：把生成的可执行文件统一放到 bin 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  

add_executable(MiniSearch src/main.cpp)


target_include_directories(MiniSearch
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/file_reader
    ${CMAKE_SOURCE_DIR}/src/query_engine
    ${CMAKE_SOURCE_DIR}/src/text_processor
)

target_link_libraries(MiniSearch
    PRIVATE
    FilePreproc
    QueryParse
    TextProcessor
)

add_custom_command(TARGET MiniSearch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:FilePreproc>"
        "$<TARGET_FILE_DIR:MiniSearch>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:QueryParse>"
        "$<TARGET_FILE_DIR:MiniSearch>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:TextProcessor>"
        "$<TARGET_FILE_DIR:MiniSearch>"
)

# 安装规则
install(TARGETS MiniSearch FilePreproc QueryParse TextProcessor
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
)

# 打包成 zip
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "MiniSearch")
set(CPACK_PACKAGE_VERSION "1.0.0")
include(CPack)