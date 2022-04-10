cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Star_Finder_Telescope)

# Specify the C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set compiler flags (Temporarily removed -Werror due to currently unimplemented functions)
set (CMAKE_CXX_FLAGS "-g3 -Os -fsanitize=address -pedantic -fno-common -Wall -Wextra -Wshadow -Wdouble-promotion -Wundef -Wconversion")

# Update the location of the executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

option(UNIT_TEST "Create unit test executable" OFF)

if(UNIT_TEST)
   add_subdirectory(test)
else()
   add_subdirectory(src)
endif()