cmake_minimum_required(VERSION 3.16)

project(qt-transport-catalogue VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg SvgWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg SvgWidgets)

qt_wrap_ui(PROJECT_UI_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/view/bus_editor.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/view/mainwindow.ui
)

file(GLOB PROJECT_SOURCES
    "*.cpp"
    "*.h"
    "*.ui"
    "*.qrc"
    "controller/*.cpp"
    "controller/*.h"
    "view/*.ui"
    "images/*.qrc"
)

set(PROJECT_SOURCES
    ${PROJECT_SOURCES}
    ${PROJECT_UI_FILES}
)

# set(PROJECT_SOURCES
#     bus_editor.ui

#     view/mainwindow.ui
#     images/resources.qrc
#     controller/bus_editor.cpp
#     controller/dialog_editor.cpp
#     main.cpp
#     controller/mainwindow.cpp
#     controller/bus_editor.h
#     controller/dialog_editor.h
#     controller/mainwindow.h
#     controller/stdafx.h
# )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qt-transport-catalogue
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qt-transport-catalogue APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qt-transport-catalogue SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qt-transport-catalogue
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(qt-transport-catalogue PRIVATE Qt${QT_VERSION_MAJOR}::Widgets model  Qt6::Svg Qt6::SvgWidgets)
target_include_directories(qt-transport-catalogue PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/model)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qt-transport-catalogue)
endif()
set_target_properties(qt-transport-catalogue PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qt-transport-catalogue
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qt-transport-catalogue)
endif()
add_subdirectory(model)
