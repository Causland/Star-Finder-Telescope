#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "PositionManager.hpp"
#include "StarTracker.hpp"
#include <algorithm>

const std::string StarTracker::NAME{"StarTracker"};

void StarTracker::start()
{
   myThread = std::thread(&StarTracker::threadLoop, this);
}

void StarTracker::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
   myThread.join();
}

void StarTracker::configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                                       static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // InformationDisplay
   myInformationDisplay = std::dynamic_pointer_cast<InformationDisplay>(
                                    subsystems[static_cast<int>(SubsystemEnum::INFORMATION_DISPLAY)]);
   if (myInformationDisplay.expired())
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Information Display");
   }
   // PositionManager
   myPositionManager = std::dynamic_pointer_cast<PositionManager>(
                                 subsystems[static_cast<int>(SubsystemEnum::POSITION_MANAGER)]);
   if (myPositionManager.expired())
   {
      Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Position Manager");
   }
}

void StarTracker::threadLoop()
{
   while (!myExitingFlag)
   {
      myHeartbeatFlag = true;
      std::unique_lock lk(myMutex);
      if (!myCondVar.wait_for(lk, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return !myCommandQueue.empty() || myExitingFlag; }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
      {
         break;
      }

      // Process each command and take the specified action
      while (!myCommandQueue.empty())
      {
         auto& generalCommand = myCommandQueue.front();
         switch (generalCommand.myCommandType)
         {
            case CommandTypeEnum::GOTO_TARGET:
            {
               auto& command = static_cast<CmdGoToTarget&>(generalCommand);
               myGpsModule->getGpsPosition(&myGpsLat, &myGpsLong, &myGpsElev);
               auto result = myStarDatabase->queryTargetPointing(command.myTargetName, std::chrono::system_clock::now(),
                                                                  myGpsLong, myGpsLat, myGpsElev);
               if (result.myStatusCode == QueryResult::SUCCESS)
               {
                  auto position = result.myPositionResults.front();
                  auto positionManager = myPositionManager.lock();
                  Logger::log(mySubsystemName, LogCodeEnum::INFO, "Issuing goto command for target=" + command.myTargetName + " (az,el)=(" + 
                                                                     std::to_string(position.first.myAzimuth) + "," + std::to_string(position.first.myElevation) + ")");
                  positionManager->updatePosition(CmdUpdatePosition(position.first.myAzimuth, position.first.myElevation));
               }
               else if (result.myStatusCode == QueryResult::INVALID_PARAM || result.myStatusCode == QueryResult::FAILURE)
               {
                  Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Goto Target Result: " + result.myLogStatement);
               }
               break;
            }
            case CommandTypeEnum::FOLLOW_TARGET:
            {
               auto& command = static_cast<CmdFollowTarget&>(generalCommand);
               myGpsModule->getGpsPosition(&myGpsLat, &myGpsLong, &myGpsElev);
               auto result = myStarDatabase->queryTargetPointingTrajectory(command.myTargetName, command.myStartTime, command.myStartTime + command.myDuration,
                                                                           myGpsLong, myGpsLat, myGpsElev);
               if (result.myStatusCode == QueryResult::SUCCESS)
               {
                  auto positionManager = myPositionManager.lock();
                  positionManager->trackTarget(&result.myPositionResults);
               }
               else if (result.myStatusCode == QueryResult::INVALID_PARAM || result.myStatusCode == QueryResult::FAILURE)
               {
                  Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Follow Target Result: " + result.myLogStatement);
               }
               break;
            }
            case CommandTypeEnum::SEARCH_TARGET:
            {
               auto& command = static_cast<CmdSearchTarget&>(generalCommand);
               QueryResult searchResult;
               if (command.myTargetName != "None")
               {
                  Logger::log(mySubsystemName, LogCodeEnum::INFO, "Searching for " + command.myTargetName);
                  searchResult = myStarDatabase->searchTargetsByName(command.myTargetName);
               }
               else if (command.mySearchRadiusInLightYears > 0)
               {
                  Logger::log(mySubsystemName, LogCodeEnum::INFO, "Searching for " + std::to_string(command.mySearchRadiusInLightYears));
                  searchResult = myStarDatabase->searchTargetsByRange(command.mySearchRadiusInLightYears);
               }
               else if (command.mySearchLuminosityInWatts > 0)
               {
                  Logger::log(mySubsystemName, LogCodeEnum::INFO, "Searching for " + std::to_string(command.mySearchLuminosityInWatts));
                  searchResult = myStarDatabase->searchTargetsByLuminosity(command.mySearchLuminosityInWatts);
               }
               else
               {
                  Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Invalid parameters for search. Given: name=" + 
                                                                        command.myTargetName + " radius=" + 
                                                                        std::to_string(command.mySearchRadiusInLightYears) + " luminosity=" +
                                                                        std::to_string(command.mySearchLuminosityInWatts));
               }

               // Check the status code and display the search results to the user
               auto informationDisplay = myInformationDisplay.lock();
               if (informationDisplay == nullptr)
               {
                  break;
               }
               switch(searchResult.myStatusCode)
               {
                  case QueryResult::SUCCESS:
                  {
                     // Each entry of the result vector is the name of a target. Format the
                     // results as a comma seperated list.
                     auto it = searchResult.mySearchResults.begin();
                     std::string displayString = *it++;
                     for (; it != searchResult.mySearchResults.end(); ++it)
                     {
                        displayString += ", ";
                        displayString += *it;
                     }
                     informationDisplay->updateSearchResults(displayString);
                     break;
                  }
                  case QueryResult::FAILURE:
                  case QueryResult::INVALID_PARAM:
                  {
                     Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Search failed with log: " + searchResult.myLogStatement);
                     informationDisplay->updateSearchResults(searchResult.myLogStatement);
                     break;
                  }
                  case QueryResult::NO_MATCH:
                  {
                     Logger::log(mySubsystemName, LogCodeEnum::WARNING, "No search matches returned from database");
                     informationDisplay->updateSearchResults("No Matches");
                     break;
                  }
                  case QueryResult::UNINITIALIZED:
                  {
                     Logger::log(mySubsystemName, LogCodeEnum::ERROR, "QueryResult is uninitialized");
                     informationDisplay->updateSearchResults("Unable to search database");
                     break;
                  }
               }
               break;
            }
            default:
            {
               Logger::log(mySubsystemName, LogCodeEnum::ERROR, "Invalid command in queue: " + generalCommand.toString());
               break;
            }
         }
         myCommandQueue.pop();
      }
   }
}

void StarTracker::pointToTarget(const CmdGoToTarget& cmd)
{
   std::scoped_lock lk(myMutex);
   myCommandQueue.push(cmd);
   myCondVar.notify_one();
}

void StarTracker::trackTarget(const CmdFollowTarget& cmd)
{
   std::scoped_lock lk(myMutex);
   myCommandQueue.push(cmd);
   myCondVar.notify_one();  
}

void StarTracker::searchForTargets(const CmdSearchTarget& cmd)
{
   std::scoped_lock lk(myMutex);
   myCommandQueue.push(cmd);
   myCondVar.notify_one();
}