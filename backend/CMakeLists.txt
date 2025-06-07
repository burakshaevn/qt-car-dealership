cmake_minimum_required(VERSION 3.16)

project(backend LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)

add_library(backend

  database_handler.h database_handler.cpp
  user.h
  table.h table.cpp
  domain.h
  edit_dialog.h edit_dialog.cpp
  products.h products.cpp
  product_card.h product_card.cpp
  cart.h cart.cpp
  floating_widgets.h floating_widgets.cpp
  user.cpp
  contract_templates.h contract_templates.cpp

)
target_link_libraries(backend Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

include(GNUInstallDirs)
install(TARGETS backend
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
