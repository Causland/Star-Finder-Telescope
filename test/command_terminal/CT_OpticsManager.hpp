#include "MinOpticsManager.hpp"

class CT_OpticsManager : public MinOpticsManager
{
public:
   std::string takePhoto(const CmdTakePhoto& cmd) override 
   {
      myTakePhotoCmd = cmd;
      myCommandReceived = true;
      return ""; 
   }
   std::string takeVideo(const CmdTakeVideo& cmd) override 
   {
      myTakeVideoCmd = cmd;
      myCommandReceived = true;
      return ""; 
   } 
   std::string takeTimelapse(const CmdTakeTimelapse& cmd) override 
   {
      myTakeTimelapseCmd = cmd;
      myCommandReceived = true;
      return ""; 
   }
   void reset()
   {
      myTakePhotoCmd = CmdTakePhoto();
      myTakeVideoCmd = CmdTakeVideo();
      myTakeTimelapseCmd = CmdTakeTimelapse();
      myCommandReceived = false;
   }

   void userChangeFocus(const CmdUserFocus& cmd)
   {
      myUserFocusCmd = cmd;
      myCommandReceived = true;
   }

   CmdTakePhoto myTakePhotoCmd{};
   CmdTakeVideo myTakeVideoCmd{};
   CmdTakeTimelapse myTakeTimelapseCmd{};
   CmdUserFocus myUserFocusCmd{};
   bool myCommandReceived{false};
};