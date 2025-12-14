cmake_minimum_required(VERSION 3.16)
project(GameEngine VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Build options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example games" OFF)

# Find SFML
find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

# Print SFML info
message(STATUS "SFML version: ${SFML_VERSION}")
message(STATUS "SFML include dir: ${SFML_INCLUDE_DIR}")

# Add subdirectories
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()