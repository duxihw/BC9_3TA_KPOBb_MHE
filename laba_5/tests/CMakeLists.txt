add_executable(grid_tests
    test_framework.cpp
    test_lab_examples.cpp
    test_grid2d.cpp
    test_grid_nd.cpp)
target_link_libraries(grid_tests PRIVATE grid::grid grid_support)
target_include_directories(grid_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
grid_set_warnings(grid_tests)
# assert() внутри Grid должны работать и в Release-сборке тестов.
if(MSVC)
    target_compile_options(grid_tests PRIVATE /UNDEBUG)
else()
    target_compile_options(grid_tests PRIVATE -UNDEBUG)
endif()

add_test(NAME grid_tests COMMAND grid_tests)
