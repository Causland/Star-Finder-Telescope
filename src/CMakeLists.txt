# Create a library with all sources (to facilitate linking during testing)
add_library(SFT
   CommandTerminal.cpp
   InformationDisplay.cpp
   Logger.cpp
   OpticsManager.cpp
   PositionManager.cpp
   StarTracker.cpp
   PropertyManager.cpp
)

# Add the include folder
target_include_directories(SFT PUBLIC
   "${CMAKE_SOURCE_DIR}/include"
   "${PROJECT_SOURCE_DIR}/util"
   "${curlpp_SOURCE_DIR}/include"
   "${sqlpp11_SOURCE_DIR}/include"
   "${tomlplusplus_SOURCE_DIR}/include"
)

# Add static libs to the executable
target_link_libraries(SFT PUBLIC
   curlpp
   sqlite3
   sqlpp11
   Threads::Threads
)

# Add, include, and link any target specific files, folders, and libraries
if (BUILD_RASPBERRYPI)
   target_sources(SFT PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/MotionController/AVRMotionController.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/GpsModule/Bn180Gps/Bn180GpsModule.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/StarDatabase/SimStarDatabase.cpp
      ${PROJECT_SOURCE_DIR}/util/serial/Serial.cpp
   )
else()
   target_sources(SFT PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/StarDatabase/SimStarDatabase.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/GpsModule/SimGpsModule.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/MotionController/SimMotionController.cpp
   )  
endif()

# Create an executable with only main and non-libSFT files 
add_executable(Star_Finder_Telescope
   main.cpp
)

target_link_libraries(Star_Finder_Telescope PUBLIC
   SFT
)
