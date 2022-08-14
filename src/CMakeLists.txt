# Add the executable to the project
add_executable(Star_Finder_Telescope
   CommandTerminal.cpp
   InformationDisplay.cpp
   Logger.cpp
   main.cpp
   OpticsManager.cpp
   PositionManager.cpp
   interfaces/StarDatabase/SimStarDatabase.cpp
   interfaces/GpsModule/SimGpsModule.cpp
   StarTracker.cpp
   interfaces/MotionController/SimMotionController.cpp
)

# Add the include folder
target_include_directories(Star_Finder_Telescope PUBLIC
   "${CMAKE_SOURCE_DIR}/include"
   "${CMAKE_BINARY_DIR}/_deps/curlpp-src/include"
   "${CMAKE_BINARY_DIR}/_deps/sqlpp11-src/include"
)

# Add static libs to the executable
target_link_libraries(Star_Finder_Telescope PUBLIC
   curlpp
   sqlite3
   sqlpp11
   Threads::Threads
)

# Add, include, and link any target specific files, folders, and libraries
if (BUILD_RASPBERRYPI)
   target_sources(Star_Finder_Telescope PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/MotionController/RPi3MotionController.cpp
   )

   target_include_directories(Star_Finder_Telescope PUBLIC
      "${CMAKE_BINARY_DIR}/_deps/wiringpi-src/wiringPi"
   )

   target_link_libraries(Star_Finder_Telescope PUBLIC
      "${CMAKE_BINARY_DIR}/_deps/wiringpi-src/wiringPi/libwiringPi.so.2.61"
   )
endif()