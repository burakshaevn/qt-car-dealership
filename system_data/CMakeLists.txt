cmake_minimum_required(VERSION 3.16)

project(system_data LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(system_data STATIC
    src/SystemData.cpp
    include/SystemData.h
)

target_include_directories(system_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)