add_library(query_engine_lib STATIC query_engine.cpp)

target_include_directories(query_engine_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../file_reader)