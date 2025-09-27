# Tests component for MVC architecture
cmake_minimum_required(VERSION 3.16)

# Находим Qt (уже найдена в родительском CMakeLists.txt)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

# Исходные файлы тестов (используем Qt Test вместо Google Test)
set(TEST_SOURCES
    SimpleTests.cpp
)

# Создаем исполняемый файл тестов
add_executable(tests ${TEST_SOURCES})

# Подключаем заголовочные файлы
target_include_directories(tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../controller/include
)

# Линкуем библиотеки
target_link_libraries(tests
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Test
    model
    controller
)

# Настраиваем компилятор
target_compile_features(tests PRIVATE cxx_std_20)

# Добавляем тесты в CTest
add_test(NAME Tests COMMAND tests)

# Настраиваем переменные окружения для тестов
set_tests_properties(Tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Группируем тесты для лучшей организации
set_tests_properties(Tests PROPERTIES
    LABELS "unit;integration;ui"
)