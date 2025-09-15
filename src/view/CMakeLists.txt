cmake_minimum_required(VERSION 3.16)

project(view LANGUAGES CXX)

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
set(VIEW_SOURCES
    mainwindow.ui
    notifications.ui

)

# Create view library
add_library(view STATIC ${VIEW_SOURCES}
    ../../resources/resources.qrc
)

target_include_directories(view PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(view PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
) 
