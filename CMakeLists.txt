cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Star_Finder_Telescope)

# Specify the C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Configure gtest environment variables
set (GTEST_HAS_PTHREAD 1)

# Add external libraries to the project
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
   curlpp
   GIT_REPOSITORY https://github.com/jpbarrette/curlpp.git
   GIT_TAG v0.8.1
)
FetchContent_MakeAvailable(curlpp)

FetchContent_Declare(
   sqlpp11
   GIT_REPOSITORY https://github.com/rbock/sqlpp11.git
   GIT_TAG 0.61
   CMAKE_ARGS -DBUILD_SQLITE3_CONNECTOR ON
)
FetchContent_MakeAvailable(sqlpp11)

# Update the location of the executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Set common compiler flags removed -Werror for now due to sqlpp
set (CMAKE_CXX_FLAGS "-g3 -Os -fsanitize=address -pedantic -fno-common -Wall -Wextra -Wshadow -Wdouble-promotion -Wundef -Wconversion -Wno-unused-parameter")

# Add subdirectories for main application and unit tests
add_subdirectory(src)
add_subdirectory(test)