#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "PositionManager.hpp"
#include "StarTracker.hpp"
#include <algorithm>

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
      LOG_ERROR("Could not cast to Information Display");
   }
   // PositionManager
   myPositionManager = std::dynamic_pointer_cast<PositionManager>(
                                 subsystems[static_cast<int>(SubsystemEnum::POSITION_MANAGER)]);
   if (myPositionManager.expired())
   {
      LOG_ERROR("Could not cast to Position Manager");
   }
}

void StarTracker::threadLoop()
{
   while (!myExitingFlag)
   {
      myHeartbeatFlag = true;
      std::unique_lock lock{myMutex};
      if (!myCondVar.wait_for(lock, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return !myCommandQueue.empty() || myExitingFlag; }))
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
         auto& cmdVariant = myCommandQueue.front();
         if (std::holds_alternative<CmdGoToTarget>(cmdVariant))
         {
            processGoTo(std::get<CmdGoToTarget>(cmdVariant));
         }
         else if (std::holds_alternative<CmdFollowTarget>(cmdVariant))
         {
            processFollow(std::get<CmdFollowTarget>(cmdVariant));
         }
         else if (std::holds_alternative<CmdSearchTarget>(cmdVariant))
         {
            processSearch(std::get<CmdSearchTarget>(cmdVariant)); 
         }
         else 
         {
            LOG_ERROR("Invalid command in queue");
            break;
         }
         myCommandQueue.pop();
      }
   }
}

void StarTracker::pointToTarget(const CmdGoToTarget& cmd)
{
   std::scoped_lock lock(myMutex);
   myCommandQueue.emplace(cmd);
   myCondVar.notify_one();
}

void StarTracker::trackTarget(const CmdFollowTarget& cmd)
{
   std::scoped_lock lock(myMutex);
   myCommandQueue.emplace(cmd);
   myCondVar.notify_one();  
}

void StarTracker::searchForTargets(const CmdSearchTarget& cmd)
{
   std::scoped_lock lock(myMutex);
   myCommandQueue.emplace(cmd);
   myCondVar.notify_one();
}

void StarTracker::processGoTo(const CmdGoToTarget& cmd)
{
   myGpsModule->getGpsPosition(&myGpsLat, &myGpsLong, &myGpsElev);
   const auto result{myStarDatabase->queryTargetPointing(cmd.myTargetName, std::chrono::system_clock::now(),
                                                         myGpsLong, myGpsLat, myGpsElev)};
   if (result.myStatusCode == QueryResult::SUCCESS)
   {
      auto position{result.myPositionResults.front()};
      auto positionManager = myPositionManager.lock();
      LOG_INFO("Issuing goto command for target=" + cmd.myTargetName + " (az,el)=(" + 
                                             std::to_string(position.first.myAzimuth) + "," + std::to_string(position.first.myElevation) + ")");
      positionManager->updatePosition(CmdUpdatePosition(Position{position.first.myAzimuth, position.first.myElevation}));
   }
   else if (result.myStatusCode == QueryResult::INVALID_PARAM || result.myStatusCode == QueryResult::FAILURE)
   {
      LOG_ERROR("Goto Target Result: " + result.myLogStatement);
   }
}

void StarTracker::processFollow(const CmdFollowTarget& cmd)
{
   myGpsModule->getGpsPosition(&myGpsLat, &myGpsLong, &myGpsElev);
   auto result = myStarDatabase->queryTargetPointingTrajectory(cmd.myTargetName, cmd.myStartTime, cmd.myStartTime + cmd.myDuration,
                                                               myGpsLong, myGpsLat, myGpsElev);
   if (result.myStatusCode == QueryResult::SUCCESS)
   {
      auto positionManager = myPositionManager.lock();
      positionManager->trackTarget(result.myPositionResults);
   }
   else if (result.myStatusCode == QueryResult::INVALID_PARAM || result.myStatusCode == QueryResult::FAILURE)
   {
      LOG_ERROR("Follow Target Result: " + result.myLogStatement);
   }
}

void StarTracker::processSearch(const CmdSearchTarget& cmd)
{
   QueryResult searchResult;
   if (cmd.myTargetName != "None")
   {
      LOG_INFO("Searching for " + cmd.myTargetName);
      searchResult = myStarDatabase->searchTargetsByName(cmd.myTargetName);
   }
   else if (cmd.mySearchRadiusInLightYears > 0)
   {
      LOG_INFO("Searching for " + std::to_string(cmd.mySearchRadiusInLightYears));
      searchResult = myStarDatabase->searchTargetsByRange(cmd.mySearchRadiusInLightYears);
   }
   else if (cmd.mySearchLuminosityInWatts > 0)
   {
      LOG_INFO("Searching for " + std::to_string(cmd.mySearchLuminosityInWatts));
      searchResult = myStarDatabase->searchTargetsByLuminosity(cmd.mySearchLuminosityInWatts);
   }
   else
   {
      LOG_ERROR("Invalid parameters for search. Given: name=" + cmd.myTargetName + " radius=" + 
                std::to_string(cmd.mySearchRadiusInLightYears) + " luminosity=" + std::to_string(cmd.mySearchLuminosityInWatts));
   }
   // Check the status code and display the search results to the user
   auto informationDisplay = myInformationDisplay.lock();
   if (informationDisplay != nullptr)
   {
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
            LOG_ERROR("Search failed with log: " + searchResult.myLogStatement);
            informationDisplay->updateSearchResults(searchResult.myLogStatement);
            break;
         }
         case QueryResult::NO_MATCH:
         {
            LOG_WARN("No search matches returned from database");
            informationDisplay->updateSearchResults("No Matches");
            break;
         }
         case QueryResult::UNINITIALIZED:
         {
            LOG_ERROR("QueryResult is uninitialized");
            informationDisplay->updateSearchResults("Unable to search database");
            break;
         }
      }
   }
}
