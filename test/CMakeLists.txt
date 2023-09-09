# Logger Test Files
set(LOGGER_SRC
   "${CMAKE_SOURCE_DIR}/src/Logger.cpp"
   logger/TestLogger.cpp
)

# Command Terminal Test Files
set(COMMAND_TERMINAL_SRC
   "${CMAKE_SOURCE_DIR}/src/CommandTerminal.cpp"
   command_terminal/TestCommandTerminal.cpp
)

# Information Manager Test Files
set(INFORMATION_DISPLAY_SRC
   "${CMAKE_SOURCE_DIR}/src/InformationDisplay.cpp"
   information_display/TestInformationDisplay.cpp
)

# Optics Manager Test Files
set(OPTICS_MANAGER_SRC
   "${CMAKE_SOURCE_DIR}/src/OpticsManager.cpp"
   optics_manager/TestOpticsManager.cpp
)

# Position Manager Test Files
set(POSITION_MANAGER_SRC
   "${CMAKE_SOURCE_DIR}/src/PositionManager.cpp"
   "${CMAKE_SOURCE_DIR}/src/PropertyManager.cpp"
   position_manager/TestPositionManager.cpp
)

# Star Tracker Test Files
set(STAR_TRACKER_SRC
   "${CMAKE_SOURCE_DIR}/src/StarTracker.cpp"
   star_tracker/TestStarTracker.cpp
)

enable_testing()

add_executable(Telescope_Unit_Test
   "${COMMAND_TERMINAL_SRC}"
   "${INFORMATION_DISPLAY_SRC}"
   "${OPTICS_MANAGER_SRC}"
   "${POSITION_MANAGER_SRC}"
   "${STAR_TRACKER_SRC}"
   "${LOGGER_SRC}"
)

# Add the include folder
target_include_directories(Telescope_Unit_Test PUBLIC
   "${CMAKE_SOURCE_DIR}/include"
   "${CMAKE_SOURCE_DIR}/include/interfaces"
   "${CMAKE_SOURCE_DIR}/test"
   "${CMAKE_SOURCE_DIR}/test/command_terminal"
   "${CMAKE_SOURCE_DIR}/test/information_display"
   "${CMAKE_SOURCE_DIR}/test/optics_manager"
   "${CMAKE_SOURCE_DIR}/test/position_manager"
   "${CMAKE_SOURCE_DIR}/test/star_tracker"
   "${curlpp_SOURCE_DIR}/include"
   "${sqlpp11_SOURCE_DIR}/include"
   "${tomlplusplus_SOURCE_DIR}/include"
)

# Add static libs to the executable
target_link_libraries(Telescope_Unit_Test PUBLIC
   GTest::gtest_main
   curlpp
   sqlite3
   sqlpp11
   Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(Telescope_Unit_Test)