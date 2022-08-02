#include "OpticsManager.hpp"
#include <algorithm>

const std::string IOpticsManager::NAME{"OpticsManager"};

void OpticsManager::start()
{
   myThread = std::thread(&OpticsManager::threadLoop, this);
}

void OpticsManager::stop()
{
   myExitingFlag = true;
   myCondVar.notify_one();
   myThread.join();
}

void OpticsManager::configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems)
{
   // Find each subsystem from the vector and store in the respective pointer
   // IInformationDisplay
   auto it = std::find_if(subsystems.begin(), subsystems.end(), 
      [](auto& subsystem){ return subsystem->getName() == IInformationDisplay::NAME; });
   if (it == subsystems.end())
   {
      myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Unable to find Information Display pointer");
   }
   else
   {
      myInformationDisplay = std::dynamic_pointer_cast<IInformationDisplay>(*it);
      if (myInformationDisplay.expired())
      {
         myLogger->log(mySubsystemName, LogCodeEnum::ERROR, "Could not cast to Information Display");
      }
   }
}

void OpticsManager::threadLoop()
{
   while (!myExitingFlag)
   {
      // Things for the optics manager to do
      // - Process requests for photos, videos, or timelapse
      //      - Photos
      //          - Focus camera using the Motion Controller
      //          - Take a photo of the target
      //          - Store photo in a known location based on time and target
       //          - Display to output about new photo (Possibly display photo to take another)
      //      - Video
      //          - Focus camera using the Motion Controller
      //          - Take a video of the target
      //          - Store video in a known location based on time and target
      //              - Possibly break up file depending on size
      //          - Display to output about video path
      //      - Timelapse
      //          - Focus camera using the Motion Controller
      //          - Take a photo of the target at specific timelapse frequency
      //          - Store photos in a known location based on timelapse/target/time
      // - Possible to focus camera based on distance to target and focal length of the camera/telescope
            myHeartbeatFlag = true;
      std::unique_lock<std::mutex> lk(myMutex);
      if (!myCondVar.wait_for(lk, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return false || myExitingFlag; }))
      {
         continue; // Heartbeat update interval timeout
      }
      if (myExitingFlag)
      {
         break;
      }
   }
}

std::string OpticsManager::takePhoto(const CmdTakePhoto& cmd)
{
   return "";
}

std::string OpticsManager::takeVideo(const CmdTakeVideo& cmd)
{
   return "";
}

std::string OpticsManager::takeTimelapse(const CmdTakeTimelapse& cmd)
{
   return "";
}

void OpticsManager::userChangeFocus(const CmdUserFocus& cmd)
{
   
}