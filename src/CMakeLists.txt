# Add the executable to the project
add_executable(Star_Finder_Telescope
   CommandTerminal.cpp
   InformationDisplay.cpp
   Logger.cpp
   main.cpp
   MotionController.cpp
   OpticsManager.cpp
   PositionManager.cpp
   StarDatabaseAdapter.cpp
   StarTracker.cpp
)

# Add the include folder
target_include_directories(Star_Finder_Telescope PUBLIC
   "${CMAKE_SOURCE_DIR}/include"
   "${CMAKE_SOURCE_DIR}/include/interfaces"
)

# Add static libs to the executable
target_link_libraries(Star_Finder_Telescope PUBLIC
   curlpp
   sqlpp11
)