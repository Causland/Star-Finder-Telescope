# Add the executable to the project
add_executable(Star_Finder_Telescope
   CommandTerminal.cpp
   InformationDisplay.cpp
   Logger.cpp
   main.cpp
   RPi3MotionController.cpp
   OpticsManager.cpp
   PositionManager.cpp
   StarDatabaseAdapter.cpp
   StarTracker.cpp
)

# Add the include folder
target_include_directories(Star_Finder_Telescope PUBLIC
   "${CMAKE_SOURCE_DIR}/include"
   "${CMAKE_BINARY_DIR}/_deps/curlpp-src/include"
   "${CMAKE_BINARY_DIR}/_deps/sqlpp11-src/include"
   "${CMAKE_SOURCE_DIR}/lib/WiringPi/include"
)

# Add static libs to the executable
target_link_libraries(Star_Finder_Telescope PUBLIC
   curlpp
   sqlite3
   sqlpp11
   "${CMAKE_SOURCE_DIR}/lib/WiringPi/libwiringPi.so.2.70"
)