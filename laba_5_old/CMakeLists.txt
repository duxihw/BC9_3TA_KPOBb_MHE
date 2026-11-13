cmake_minimum_required(VERSION 3.20)

project(GridLab
    VERSION 1.0
    DESCRIPTION "Лабораторная работа: управление ресурсами (шаблон Grid)"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(GRID_BUILD_TESTS "Собирать тесты" ON)
option(GRID_BUILD_BENCH "Собирать бенчмарк" ON)
option(GRID_BUILD_DEMO  "Собирать демо" ON)
option(GRID_SANITIZE    "AddressSanitizer + UBSan (GCC/Clang, не MinGW)" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

function(grid_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion
                                                 -Wno-sign-conversion)
    endif()
endfunction()

if(GRID_SANITIZE AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# ---------------------------------------------------------------- библиотека
# Шаблоны Grid целиком в заголовках -> INTERFACE-библиотека (нечего компилировать
# заранее: код появляется при инстанцировании с конкретным T).
add_library(grid INTERFACE)
add_library(grid::grid ALIAS grid)
target_include_directories(grid INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(grid INTERFACE cxx_std_20)

# Вспомогательная библиотека: тип-счётчик Tracked (обычный .hpp + .cpp).
add_library(grid_support STATIC support/src/tracked.cpp)
target_include_directories(grid_support PUBLIC ${PROJECT_SOURCE_DIR}/support/include)
target_compile_features(grid_support PUBLIC cxx_std_20)
grid_set_warnings(grid_support)

if(GRID_BUILD_DEMO)
    add_subdirectory(apps/demo)
endif()
if(GRID_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(GRID_BUILD_BENCH)
    add_subdirectory(bench)
endif()
