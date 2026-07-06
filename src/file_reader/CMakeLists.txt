add_library(file_reader_lib STATIC file_reader.cpp)

target_include_directories(file_reader_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})