cmake_minimum_required(VERSION 3.16)

project(model VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

add_library(model
    database_manager.h
    domain.h
    geo.h
    graph.h
    json.h
    json_builder.h
    json_reader.h
    map_renderer.h
    ranges.h
    request_handler.h
    router.h
    svg.h
    transport_catalogue.h
    transport_router.h
    database_manager.cpp
    domain.cpp
    json.cpp
    json_builder.cpp
    json_reader.cpp
    map_renderer.cpp
    request_handler.cpp
    svg.cpp
    transport_catalogue.cpp
    transport_router.cpp
)
target_link_libraries(model Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

include(GNUInstallDirs)
install(TARGETS model
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
