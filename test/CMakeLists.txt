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

# Motion Controller Test Files
if (BUILD_RASPBERRYPI)
   set(MOTION_CONTROLLER_SRC
      "${CMAKE_SOURCE_DIR}/src/RPi3MotionController.cpp"
      motion_controller/TestMotionController.cpp
   )
endif()


# Optics Manager Test Files
set(OPTICS_MANAGER_SRC
   "${CMAKE_SOURCE_DIR}/src/OpticsManager.cpp"
   optics_manager/TestOpticsManager.cpp
)

# Position Manager Test Files
set(POSITION_MANAGER_SRC
   "${CMAKE_SOURCE_DIR}/src/PositionManager.cpp"
   position_manager/TestPositionManager.cpp
)

# Star Database Test Files
set(STAR_DATABASE_SRC
   "${CMAKE_SOURCE_DIR}/src/StarDatabaseAdapter.cpp"
   star_database/TestStarDatabase.cpp
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
   "${MOTION_CONTROLLER_SRC}"
   "${OPTICS_MANAGER_SRC}"
   "${POSITION_MANAGER_SRC}"
   "${STAR_DATABASE_SRC}"
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
   "${CMAKE_SOURCE_DIR}/test/motion_controller"
   "${CMAKE_SOURCE_DIR}/test/optics_manager"
   "${CMAKE_SOURCE_DIR}/test/position_manager"
   "${CMAKE_SOURCE_DIR}/test/star_tracker"
   "${CMAKE_BINARY_DIR}/_deps/curlpp-src/include"
   "${CMAKE_BINARY_DIR}/_deps/sqlpp11-src/include"
)

# Add static libs to the executable
target_link_libraries(Telescope_Unit_Test PUBLIC
   GTest::gtest_main
   curlpp
   sqlite3
   sqlpp11
   Threads::Threads
)

# Include and link any target specific folders and libraries
if (BUILD_RASPBERRYPI)
   target_include_directories(Telescope_Unit_Test PUBLIC
      "${CMAKE_BINARY_DIR}/_deps/wiringpi-src/wiringPi"
   )

   target_link_libraries(Telescope_Unit_Test PUBLIC
      "${CMAKE_BINARY_DIR}/_deps/wiringpi-src/wiringPi/libwiringPi.so.2.61"
   )
endif()

include(GoogleTest)
gtest_discover_tests(Telescope_Unit_Test)