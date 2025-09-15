cmake_minimum_required(VERSION 3.16)

project(controller LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt dependencies
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)

# Source files
set(CONTROLLER_SOURCES
    src/mainwindow.cpp
    include/mainwindow.h
    src/notifications_handler.cpp
    include/notifications_handler.h


)

# Create controller library
add_library(controller STATIC ${CONTROLLER_SOURCES})

target_include_directories(controller PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../model/include
)

target_link_libraries(controller PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    model
) 
