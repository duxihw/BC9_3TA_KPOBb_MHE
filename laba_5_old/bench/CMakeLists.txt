add_executable(grid_bench
    src/main.cpp
    src/alloc_counter.cpp
    src/csv_writer.cpp
    src/timer.cpp
    src/experiments.cpp)
target_include_directories(grid_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(grid_bench PRIVATE grid::grid grid_support)
grid_set_warnings(grid_bench)
get_filename_component(GRID_LABS_DIR "${PROJECT_SOURCE_DIR}/.." ABSOLUTE)
set(GRID_DATA_DIR "${GRID_LABS_DIR}/data/laba_5")
set(GRID_PLOTS_DIR "${GRID_LABS_DIR}/plots/laba_5")

target_compile_definitions(grid_bench PRIVATE GRID_DEFAULT_DATA_DIR="${GRID_DATA_DIR}")

add_custom_target(run_bench
    COMMAND grid_bench --out "${GRID_DATA_DIR}"
    DEPENDS grid_bench USES_TERMINAL VERBATIM
    COMMENT "Запуск бенчмарка (CSV -> ../data/laba_5/)")

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
    add_custom_target(analyze
        COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/python/analyze.py"
                --data "${GRID_DATA_DIR}" --plots "${GRID_PLOTS_DIR}"
        USES_TERMINAL VERBATIM
        COMMENT "Анализ CSV и построение графиков")
endif()
