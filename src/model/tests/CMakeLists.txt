find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(tests SimpleTests.cpp)
target_link_libraries(tests PRIVATE Qt6::Test controller)

add_test(NAME tests COMMAND tests)
set_tests_properties(tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
