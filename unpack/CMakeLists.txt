cmake_minimum_required(VERSION 3.1)
project(mmarch-unpack)

set(MMARCH_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../c)
add_subdirectory(${MMARCH_LIB_PATH} mmarch)
include_directories(${MMARCH_LIB_PATH})

set(SOURCES unpack.c)

add_executable(mmarch-unpack ${SOURCES})
target_link_libraries(mmarch-unpack mmarch)
target_compile_options(mmarch-unpack PRIVATE -Wall -pedantic)
