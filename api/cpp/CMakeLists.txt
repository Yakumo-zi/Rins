cmake_minimum_required(VERSION 3.20)
project(example_cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(../../reactor/include ./src ../../base/proto)
aux_source_directory(../../base/proto PROTO)
aux_source_directory(src SOURCES)
aux_source_directory(example EXAMPLE)
link_directories(../../reactor/lib)

add_executable(example_cpp ${SOURCES} ${PROTO} ${EXAMPLE})
target_link_libraries(example_cpp PUBLIC reactor pthread mysqlclient protobuf)
target_include_directories(example_cpp PUBLIC src)