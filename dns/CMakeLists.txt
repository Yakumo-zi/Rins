cmake_minimum_required(VERSION 3.20.0)
project(rins_dns)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
aux_source_directory(src SOURCES)
aux_source_directory(../base/proto PROTO)
include_directories(../reactor/include ../base/proto)
link_directories(../reactor/lib)

add_executable(dns ${SOURCES} ${PROTO})
target_link_libraries(dns PUBLIC reactor pthread mysqlclient protobuf)
target_include_directories(dns PUBLIC include)

aux_source_directory(tests TEST)
list(LENGTH TEST TEST_LENGTH)
list(APPEND NUMBERS 0)

foreach(NUM RANGE 1 ${TEST_LENGTH})
    list(APPEND NUMBERS ${NUM})
endforeach()

message(${NUMBERS})

foreach(CASE NUM IN ZIP_LISTS TEST TEST_LENGTH)
    add_executable("test-${NUM}" ${CASE} ${PROTO})
    target_link_libraries("test-${NUM}" PUBLIC reactor pthread mysqlclient protobuf)
    target_include_directories("test-${NUM}" PUBLIC include)
endforeach()
