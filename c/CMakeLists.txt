cmake_minimum_required(VERSION 3.1)
project(mmarch-unpack)

set(SOURCES mmarch.c)

add_library(mmarch STATIC ${SOURCES}) #use BUILD_SHARED_LIBS to build .so
