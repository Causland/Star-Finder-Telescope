cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Star_Finder_Telescope)

# Specify the C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Configure gtest environment variables
set (GTEST_HAS_PTHREAD 1)

# Add subdirectories for libraries before custom compiler flags
add_subdirectory(lib/curlpp)
add_subdirectory(lib/googletest)

# Update the location of the executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Set common compiler flags
set (CMAKE_CXX_FLAGS "-g3 -Os -fsanitize=address -pedantic -fno-common -Wall -Werror -Wextra -Wshadow -Wdouble-promotion -Wundef -Wconversion -Wno-unused-parameter")

# Add subdirectories for main application and unit tests
add_subdirectory(src)
add_subdirectory(test)