cmake_minimum_required(VERSION 3.21)

###########################################################
# Define options
###########################################################
option (BUILD_RASPBERRYPI "Build for Raspberry Pi with specific components" OFF)


###########################################################
# Set the project name
###########################################################
project(Star_Finder_Telescope)


###########################################################
# Set compiler/linker options and build settings
###########################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if (BUILD_RASPBERRYPI)
   add_compile_options(-DRASPBERRY_PI)
endif()


###########################################################
# Fetch common external libraries
###########################################################
include(FetchContent)
include(ExternalProject)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

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

FetchContent_Declare(
   tomlplusplus
   GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
   GIT_TAG v3.2.0
)
FetchContent_MakeAvailable(tomlplusplus)

# Configure gtest environment variables
set (GTEST_HAS_PTHREAD 1)

FetchContent_Declare(
   googletest
   GIT_REPOSITORY https://github.com/google/googletest.git
   GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


###########################################################
# Add a documentation target with Doxygen
###########################################################
find_package(Doxygen 
            REQUIRED dot
            OPTIONAL_COMPONENTS mscgen dia)
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
set(DOXYGEN_TAB_SIZE 3)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_EXTRACT_PRIV_VIRTUAL YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_FILE_PATTERNS *.c;*.cpp;*.c++;*.h;*.hxx;*.hpp;*.h++;*.markdown;*.md)
doxygen_add_docs(docs
   include/
   src/
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


###########################################################
# Set compiler flags after building fetched includes to 
# minimize warnings from external library code
###########################################################
set (CMAKE_CXX_FLAGS "-g3 -O1 -fsanitize=address -pedantic -fno-common -Wall -Wextra -Wshadow -Wdouble-promotion -Wundef -Wconversion -Wno-unused-parameter")


###########################################################
# Add the source directory
###########################################################
add_subdirectory(src)


###########################################################
# Add the unit test directory
###########################################################
enable_testing()
add_subdirectory(test)

