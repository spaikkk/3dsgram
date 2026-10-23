cmake_minimum_required(VERSION 3.25)
project(inih_lib LANGUAGES C)
set(CMAKE_C_STANDARD 11)  

add_library(inih STATIC ini.c)
target_include_directories(inih PUBLIC ${CMAKE_CURRENT_LIST_DIR})
