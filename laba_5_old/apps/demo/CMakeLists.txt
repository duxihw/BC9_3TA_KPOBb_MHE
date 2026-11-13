add_executable(grid_demo main.cpp)
target_link_libraries(grid_demo PRIVATE grid::grid grid_support)
grid_set_warnings(grid_demo)
