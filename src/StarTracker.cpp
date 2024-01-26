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
         auto& cmdVariant{myCommandQueue.front()};
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
   std::scoped_lock lock{myMutex};
   myCommandQueue.emplace(cmd);
   myCondVar.notify_one();
}

void StarTracker::trackTarget(const CmdFollowTarget& cmd)
{
   std::scoped_lock lock{myMutex};
   myCommandQueue.emplace(cmd);
   myCondVar.notify_one();  
}

void StarTracker::searchForTargets(const CmdSearchTarget& cmd)
{
   std::scoped_lock lock{myMutex};
   myCommandQueue.emplace(cmd);
   myCondVar.notify_one();
}

void StarTracker::processGoTo(const CmdGoToTarget& cmd)
{
   myGpsModule->getGpsPosition(&myGpsPosition);
   const auto result{myStarDatabase->queryTargetPointing(cmd.myTargetName, std::chrono::system_clock::now(), myGpsPosition)};
   if (result.myStatus == QueryResultEnum::SUCCESS)
   {
      const auto& traj{std::get<QueryResult::QueryRsltTrajectory>(result.myQueryResults)};
      const auto& pos{traj.front().myPosition};

      auto positionManager{myPositionManager.lock()};
      if (positionManager != nullptr)
      {
         LOG_INFO("Issuing goto command for target=" + cmd.myTargetName + " (az,el)=(" + 
                      std::to_string(pos.myAzimuth) + "," + std::to_string(pos.myElevation) + ")");
         positionManager->updatePosition(CmdUpdatePosition(pos));
      }
   }
   else if (result.myStatus == QueryResultEnum::INVALID_PARAM || result.myStatus == QueryResultEnum::FAILURE)
   {
      LOG_ERROR("Goto Target Result: " + std::get<std::string>(result.myQueryResults));
   }
}

void StarTracker::processFollow(const CmdFollowTarget& cmd)
{
   myGpsModule->getGpsPosition(&myGpsPosition);
   auto result{myStarDatabase->queryTargetPointingTrajectory(cmd.myTargetName, cmd.myStartTime, cmd.myDuration, myGpsPosition)};
   if (result.myStatus == QueryResultEnum::SUCCESS)
   {
      auto positionManager{myPositionManager.lock()};
      if (positionManager != nullptr)
      {
         positionManager->trackTarget(std::get<QueryResult::QueryRsltTrajectory>(result.myQueryResults));
      }
   }
   else if (result.myStatus == QueryResultEnum::INVALID_PARAM || result.myStatus == QueryResultEnum::FAILURE)
   {
      LOG_ERROR("Follow Target Result: " + std::get<std::string>(result.myQueryResults));
   }
}

void StarTracker::processSearch(const CmdSearchTarget& cmd)
{
   QueryResult queryResult{QueryResultEnum::FAILURE};
   if (cmd.myTargetName != "None")
   {
      LOG_INFO("Searching for " + cmd.myTargetName);
      queryResult = myStarDatabase->searchTargetsByName(cmd.myTargetName);
   }
   else if (cmd.mySearchRadiusInLightYears > 0)
   {
      LOG_INFO("Searching for " + std::to_string(cmd.mySearchRadiusInLightYears));
      queryResult = myStarDatabase->searchTargetsByRange(cmd.mySearchRadiusInLightYears);
   }
   else if (cmd.mySearchLuminosityInWatts > 0)
   {
      LOG_INFO("Searching for " + std::to_string(cmd.mySearchLuminosityInWatts));
      queryResult = myStarDatabase->searchTargetsByLuminosity(cmd.mySearchLuminosityInWatts);
   }
   else
   {
      LOG_ERROR("Invalid parameters for search. Given: name=" + cmd.myTargetName + " radius=" + 
                std::to_string(cmd.mySearchRadiusInLightYears) + " luminosity=" + std::to_string(cmd.mySearchLuminosityInWatts));
   }
   // Check the status code and display the search results to the user
   auto informationDisplay{myInformationDisplay.lock()};
   if (informationDisplay != nullptr)
   {
      switch(queryResult.myStatus)
      {
         case QueryResultEnum::SUCCESS:
         {
            // Each entry of the result vector is the name of a target. Format the
            // results as a comma seperated list.
            const auto& searchResult{std::get<QueryResult::QueryRsltSearch>(queryResult.myQueryResults)};
            auto iter{searchResult.begin()};
            std::string displayString{*iter++};
            for (; iter != searchResult.end(); ++iter)
            {
               displayString += "\n" + *iter;
            }
            informationDisplay->updateSearchResults(displayString);
            break;
         }
         case QueryResultEnum::FAILURE:
         case QueryResultEnum::INVALID_PARAM:
         {
            const auto& logStatement{std::get<std::string>(queryResult.myQueryResults)};
            LOG_ERROR("Search failed: " + logStatement);
            informationDisplay->updateSearchResults(logStatement);
            break;
         }
         case QueryResultEnum::NO_MATCH:
         {
            LOG_WARN("No search matches returned from database");
            informationDisplay->updateSearchResults("No Matches");
            break;
         }
      }
   }
}
