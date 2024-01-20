#include "InformationDisplay.hpp"
#include "Logger.hpp"
#include "OpticsManager.hpp"
#include <algorithm>

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

void OpticsManager::configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
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
      std::unique_lock<std::mutex> lock{myMutex};
      if (!myCondVar.wait_for(lock, HEARTBEAT_UPDATE_INTERVAL_MS, [this](){ return myExitingFlag.load(); }))
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
