cmake_minimum_required(VERSION 3.16)

project(model LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt dependencies
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)

# PostgreSQL
find_package(PostgreSQL REQUIRED)

# Project structure
set(MODEL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODEL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES 
    "${MODEL_SOURCE_DIR}/*.cpp"
    "${MODEL_SOURCE_DIR}/*.cc"
)

# Header files
file(GLOB_RECURSE HEADERS 
    "${MODEL_INCLUDE_DIR}/*.h"
    "${MODEL_INCLUDE_DIR}/*.hpp"
)

# Main library
add_library(model 
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(model PUBLIC
    $<BUILD_INTERFACE:${MODEL_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(model PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    ${PostgreSQL_LIBRARY}
)

target_include_directories(model PUBLIC
    ${PostgreSQL_INCLUDE_DIR}
)

# Installation
include(GNUInstallDirs)
install(TARGETS model
    EXPORT modelTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${MODEL_INCLUDE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT modelTargets
    FILE modelTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/model
)
