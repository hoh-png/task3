# 添加可执行文件
add_executable(mini_search_benchmark
    benchmark_search.cpp
    # 添加其他 benchmark 源文件
)

# 添加包含目录（关键！）
target_include_directories(mini_search_benchmark
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src          # 让 benchmark 能找到 src/ 下的头文件
    ${CMAKE_SOURCE_DIR}/src/query_engine
    ${CMAKE_SOURCE_DIR}/src/text_processor
    ${CMAKE_SOURCE_DIR}/src/file_reader
)

# 链接 Google Benchmark 库
target_link_libraries(mini_search_benchmark
    PRIVATE
    benchmark::benchmark          # Google Benchmark
    query_engine_lib              # 您的查询引擎库（自动提供 query_engine.h 路径）
    text_processor_lib            # 您的文本处理库（自动提供 text_processor.h 路径）
    file_reader_lib               # 您的文件读取库（自动提供 file_reader.h 路径）
    Threads::Threads              
)

# 如果使用 pthread
find_package(Threads REQUIRED)
target_link_libraries(mini_search_benchmark
    PRIVATE
    Threads::Threads
)

# 设置 C++ 标准
target_compile_features(mini_search_benchmark
    PRIVATE
    cxx_std_17
)