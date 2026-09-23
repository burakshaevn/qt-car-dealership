cmake_minimum_required(VERSION 3.19)

project(qt-car-dealership VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Sql)
qt_standard_project_setup()

# All SQL (migrations, seed data, queries) lives in system_data/sql and is
# compiled into Qt resources. C++ code refers to statements by name only.
add_subdirectory(system_data)

# --- Model: SQLite access, repositories, item models ------------------------
set(MODEL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/AdminRepository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/AdminTableModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/ClientRepository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/ContractTemplates.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/DatabaseHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/DatabaseMigrator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/ProductListModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/ProductRepository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/ReferenceDataRepository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/src/RequestRepository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/AdminRepository.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/AdminTableModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/ClientRepository.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/ContractTemplates.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/DatabaseHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/DatabaseMigrator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/PriceFormatter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/ProductListModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/ProductRepository.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/PurchaseMethod.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/ReferenceDataRepository.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/RequestRepository.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/UserInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include/UserRole.h
)

add_library(model STATIC ${MODEL_SOURCES})
target_include_directories(model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/model/include)
target_link_libraries(model PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql system_data)
target_compile_definitions(model PUBLIC
    CAR_IMAGES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/cars"
)

# --- View: theme, reusable widgets and passive pages ------------------------
set(VIEW_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/FlowLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/ProductCardDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/RecordEditDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/ThemeManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/UiKit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/AdminPage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/CatalogPage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/LoginPage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/NavigationSidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/NotificationsPage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/ProductPage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/ProfilePage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/src/pages/TopBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/FlowLayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/ProductCardDelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/RecordEditDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/ThemeManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/UiKit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/AdminPage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/CatalogPage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/LoginPage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/NavigationSidebar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/NotificationsPage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/ProductPage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/ProfilePage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include/pages/TopBar.h
)

add_library(view STATIC ${VIEW_SOURCES})
target_include_directories(view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/view/include)
target_link_libraries(view PUBLIC Qt6::Widgets model)

# --- Controller: composition root and page <-> repository wiring ------------
set(CONTROLLER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/AdminController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/AppServices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/AuthController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/CatalogController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/MainWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/NotificationsController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/RequestController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/SettingsForm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/src/UserSession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/AdminController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/AppServices.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/AuthController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/CatalogController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/MainWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/NotificationsController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/RequestController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/SettingsForm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include/UserSession.h
)

add_library(controller STATIC ${CONTROLLER_SOURCES})
target_include_directories(controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/include)
target_link_libraries(controller PUBLIC Qt6::Widgets Qt6::Sql model view)

# --- Application -------------------------------------------------------------
qt_add_executable(qt-car-dealership main.cpp resources/resources.qrc)
target_compile_definitions(qt-car-dealership PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(qt-car-dealership PRIVATE controller)
set_target_properties(qt-car-dealership PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/model/tests)
endif()

include(GNUInstallDirs)
install(TARGETS qt-car-dealership
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
