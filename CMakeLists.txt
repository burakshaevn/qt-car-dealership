cmake_minimum_required(VERSION 3.16)

project(qt-car-dealership VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Указываем Qt, где искать UI файлы
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/view)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# Model
set(MODEL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/model)
set(MODEL_SOURCES
    ${MODEL_DIR}/src/ContractTemplates.cpp
    ${MODEL_DIR}/src/AdminTableModel.cpp
    ${MODEL_DIR}/src/DatabaseHandler.cpp
    ${MODEL_DIR}/src/AdminRecordEditDialog.cpp
    ${MODEL_DIR}/src/FloatingNavigationWidget.cpp
    ${MODEL_DIR}/src/ProductListModel.cpp
    ${MODEL_DIR}/src/PurchaseMethodListModel.cpp
    ${MODEL_DIR}/src/ProductRepository.cpp
    ${MODEL_DIR}/src/AdminTableWidget.cpp
    ${MODEL_DIR}/src/ThemeStyleProvider.cpp
)
set(MODEL_HEADERS
    ${MODEL_DIR}/include/ContractTemplates.h
    ${MODEL_DIR}/include/AdminTableModel.h
    ${MODEL_DIR}/include/DatabaseHandler.h
    ${MODEL_DIR}/include/UserRole.h
    ${MODEL_DIR}/include/UserInfo.h
    ${MODEL_DIR}/include/PriceFormatter.h
    ${MODEL_DIR}/include/AdminRecordEditDialog.h
    ${MODEL_DIR}/include/FloatingNavigationWidget.h
    ${MODEL_DIR}/include/ProductListModel.h
    ${MODEL_DIR}/include/PurchaseMethod.h
    ${MODEL_DIR}/include/PurchaseMethodListModel.h
    ${MODEL_DIR}/include/ProductRepository.h
    ${MODEL_DIR}/include/AdminTableWidget.h
    ${MODEL_DIR}/include/ThemeStyleProvider.h
)

add_library(model ${MODEL_SOURCES} ${MODEL_HEADERS})

target_compile_definitions(model PUBLIC
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)

target_include_directories(model PUBLIC
    ${MODEL_DIR}/include
    ${PostgreSQL_INCLUDE_DIR}
)

target_link_libraries(model PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    ${PostgreSQL_LIBRARY}
)

# Controller ---
set(VIEW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/view)
set(CONTROLLER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/controller)
set(CONTROLLER_SOURCES
    ${CONTROLLER_DIR}/src/AppServices.cpp
    ${CONTROLLER_DIR}/src/UserSession.cpp
    ${CONTROLLER_DIR}/src/MainWindow.cpp
    ${CONTROLLER_DIR}/src/CatalogController.cpp
    ${CONTROLLER_DIR}/src/ProfileController.cpp
    ${CONTROLLER_DIR}/src/AuthController.cpp
    ${CONTROLLER_DIR}/src/NotificationsController.cpp
    ${CONTROLLER_DIR}/src/AdminTableController.cpp
    ${CONTROLLER_DIR}/src/PurchaseMethodsController.cpp
    ${CONTROLLER_DIR}/src/PurchaseRequestStrategy.cpp
    ${CONTROLLER_DIR}/src/SelectedCarActionStrategy.cpp
    ${CONTROLLER_DIR}/src/SettingsForm.cpp
    ${CONTROLLER_DIR}/src/NotificationsHandler.cpp
    ${CONTROLLER_DIR}/include/AppServices.h
    ${CONTROLLER_DIR}/include/UserSession.h
    ${CONTROLLER_DIR}/include/MainWindow.h
    ${CONTROLLER_DIR}/include/CatalogController.h
    ${CONTROLLER_DIR}/include/ProfileController.h
    ${CONTROLLER_DIR}/include/AuthController.h
    ${CONTROLLER_DIR}/include/NotificationsController.h
    ${CONTROLLER_DIR}/include/AdminTableController.h
    ${CONTROLLER_DIR}/include/PurchaseMethodsController.h
    ${CONTROLLER_DIR}/include/PurchaseRequestStrategy.h
    ${CONTROLLER_DIR}/include/SelectedCarActionStrategy.h
    ${CONTROLLER_DIR}/include/SettingsForm.h
    ${CONTROLLER_DIR}/include/NotificationsHandler.h
)
add_library(controller STATIC ${CONTROLLER_SOURCES})
target_include_directories(controller PUBLIC
    ${CONTROLLER_DIR}/include
    ${MODEL_DIR}/include
    ${VIEW_DIR}/include
)
target_link_libraries(controller PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    model
    view
)

# --- View (исходники из папки src/view) ---
set(VIEW_SOURCES
    ${VIEW_DIR}/MainWindow.ui
    ${VIEW_DIR}/notifications.ui
    ${VIEW_DIR}/PurchaseRequestForm.ui
    ${VIEW_DIR}/RentalRequestForm.ui
    ${VIEW_DIR}/TestDriveRequestForm.ui
    ${VIEW_DIR}/CreditRequestForm.ui
    ${VIEW_DIR}/src/ProductCardDelegate.cpp
    ${VIEW_DIR}/src/PurchaseMethodCardDelegate.cpp
)
set(VIEW_HEADERS
    ${VIEW_DIR}/include/ProductCardDelegate.h
    ${VIEW_DIR}/include/PurchaseMethodCardDelegate.h
)
add_library(view STATIC ${VIEW_SOURCES} ${VIEW_HEADERS})
target_include_directories(view PUBLIC
    ${VIEW_DIR}
    ${VIEW_DIR}/include
    ${MODEL_DIR}/include
)
target_link_libraries(view PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

# Add tests (optional - only if testing is enabled)
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/model/tests)
endif()

# Main application sources
set(PROJECT_SOURCES
  main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qt-car-dealership
      MANUAL_FINALIZATION
      ${PROJECT_SOURCES}
      resources/resources.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qt-car-dealership APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qt-car-dealership SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qt-car-dealership
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Link libraries
target_link_libraries(qt-car-dealership PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Sql 
    model
    controller
    view
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qt-car-dealership)
endif()
set_target_properties(qt-car-dealership PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qt-car-dealership
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qt-car-dealership)
endif()

add_subdirectory(system_data)