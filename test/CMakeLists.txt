# Logger Test Files
set(LOGGER_SRC
   "${CMAKE_SOURCE_DIR}/src/Logger.cpp"
)

# Command Terminal Test Files
set(COMMAND_TERMINAL_SRC
   "${CMAKE_SOURCE_DIR}/src/CommandTerminal.cpp"
   TestCommandTerminal.cpp
)
set(COMMAND_TERMINAL_HDRS
   MinMotionController.hpp
   MinOpticsManager.hpp
   MinPositionManager.hpp
   MinStarTracker.hpp
) 

# Information Manager Test Files
set(INFORMATION_DISPLAY_SRC
   "${CMAKE_SOURCE_DIR}/src/InformationDisplay.cpp"
)

# Motion Controller Test Files
set(MOTION_CONTROLLER_SRC
   "${CMAKE_SOURCE_DIR}/src/MotionController.cpp"
)

# Optics Manager Test Files
set(OPTICS_MANAGER_SRC
   "${CMAKE_SOURCE_DIR}/src/OpticsManager.cpp"
)

# Position Manager Test Files
set(POSITION_MANAGER_SRC
   "${CMAKE_SOURCE_DIR}/src/PositionManager.cpp"
)

# Star Database Test Files
set(STAR_DATABASE_SRC
   "${CMAKE_SOURCE_DIR}/src/StarDatabase.cpp"
)

# Star Tracker Test Files
set(STAR_TRACKER_SRC
   "${CMAKE_SOURCE_DIR}/src/StarTracker.cpp"
)

add_executable(Telescope_Unit_Test
   "${COMMAND_TERMINAL_SRC}"
   "${COMMAND_TERMINAL_HDRS}"
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
)

# Add static libs to the executable
target_link_libraries(Telescope_Unit_Test PUBLIC
   gtest_main
)