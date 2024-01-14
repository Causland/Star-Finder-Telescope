include(GoogleTest)

# Create a test executable for each test suite, then use gtest discover to
# register with ctest. Test executables must contain suffix "_Test" and
# are excluded from the "all" target
function(create_test)
   set(oneValueArgs COMPONENT_NAME)
   set(multiValueArgs SRCS INCLUDE_DIRS LIBS)
   cmake_parse_arguments(TEST "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

   add_executable("${TEST_COMPONENT_NAME}_Test" EXCLUDE_FROM_ALL
      ${TEST_SRCS}
   )

   target_include_directories("${TEST_COMPONENT_NAME}_Test" PUBLIC
      ${PROJECT_SOURCE_DIR}/include
      ${PROJECT_SOURCE_DIR}/include/interfaces
      ${PROJECT_SOURCE_DIR}/test
      ${TEST_INCLUDE_DIRS}
   )

   target_link_libraries("${TEST_COMPONENT_NAME}_Test" PUBLIC
      GTest::gtest_main
      Threads::Threads
      SFT
      ${TEST_LIBS}
   )

   gtest_discover_tests("${TEST_COMPONENT_NAME}_Test"
      DISCOVERY_MODE PRE_TEST)
endfunction()

# Logger
create_test(COMPONENT_NAME Logger
   SRCS
      ${PROJECT_SOURCE_DIR}/src/Logger.cpp
      ${CMAKE_CURRENT_LIST_DIR}/logger/TestLogger.cpp
   INCLUDE_DIRS
      ${PROJECT_SOURCE_DIR}/test/logger
)

# Command Terminal
create_test(COMPONENT_NAME CommandTerminal
   SRCS
      ${PROJECT_SOURCE_DIR}/src/CommandTerminal.cpp
      ${CMAKE_CURRENT_LIST_DIR}/command_terminal/TestCommandTerminal.cpp
   INCLUDE_DIRS
      ${CMAKE_CURRENT_LIST_DIR}/command_terminal
)

add_custom_target(tests)
add_dependencies(tests
   Logger_Test
   CommandTerminal_Test
)

