cmake_minimum_required(VERSION 3.10)
project(app)

include_directories(includes/)

add_library(eru lib/eru.c)
install(TARGETS eru)

add_subdirectory(examples/)
