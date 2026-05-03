cmake_minimum_required(VERSION 3.10)
project(eru)

add_library(eru SHARED lib/eru.c)
target_include_directories(eru PUBLIC includes/)
install(TARGETS eru)

add_subdirectory(examples/)
