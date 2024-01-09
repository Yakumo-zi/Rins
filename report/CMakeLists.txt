cmake_minimum_required(VERSION 3.20.0)
project(report)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(../reactor/include ./include ../base/proto)
aux_source_directory(../base/proto PROTO)
aux_source_directory(src SOURCES)
link_directories(../reactor/lib)

add_executable(report ${SOURCES} ${PROTO})
target_link_libraries(report PUBLIC reactor pthread mysqlclient protobuf)
target_include_directories(report PUBLIC include)

aux_source_directory(tests TEST)
list(LENGTH TEST TEST_LENGTH)

if(${TEST_LENGTH} GREATER 0)
    list(APPEND NUMBERS 0)

    foreach(NUM RANGE 1 ${TEST_LENGTH})
        list(APPEND NUMBERS ${NUM})
    endforeach()
    foreach(CASE NUM IN ZIP_LISTS TEST TEST_LENGTH)
        add_executable("re-test-${NUM}" ${CASE} ${PROTO})
        target_link_libraries("re-test-${NUM}" PUBLIC reactor pthread mysqlclient protobuf)
        target_include_directories("re-test-${NUM}" PUBLIC include)
    endforeach()
endif()