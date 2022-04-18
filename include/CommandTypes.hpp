#ifndef COMMAND_TYPES_HPP
#define COMMAND_TYPES_HPP

#include <chrono>
#include <string>

enum class CommandTypeEnum
{
   TAKE_PHOTO,
   TAKE_VIDEO,
   TAKE_TIMELAPSE,
   USER_MOVE,
   USER_FOCUS,
   FOLLOW_TARGET,
   GOTO_TARGET,
   SEARCH_TARGET,
   CALIBRATE,
   UNKNOWN,
};

struct Command
{
   explicit Command(const CommandTypeEnum& commandType) : myCommandType(commandType) {}
   CommandTypeEnum myCommandType{CommandTypeEnum::UNKNOWN};
};

struct CmdTakePhoto : Command
{
   CmdTakePhoto() : Command(CommandTypeEnum::TAKE_PHOTO) {} 
   explicit CmdTakePhoto(std::string photoName) : 
      Command(CommandTypeEnum::TAKE_PHOTO), 
      myPhotoName(std::move(photoName)) {}
   
   std::string myPhotoName{"None"};
};

struct CmdTakeVideo : Command
{
   CmdTakeVideo() : Command(CommandTypeEnum::TAKE_VIDEO) {}
   explicit CmdTakeVideo(std::string videoName) : 
      Command(CommandTypeEnum::TAKE_VIDEO),
      myVideoName(std::move(videoName)) {}

   std::string myVideoName{"None"};
};

struct CmdTakeTimelapse : Command
{
   CmdTakeTimelapse() : Command(CommandTypeEnum::TAKE_TIMELAPSE) {}
   explicit CmdTakeTimelapse(std::string timelapseName) : 
      Command(CommandTypeEnum::TAKE_TIMELAPSE),
      myTimelapseName(std::move(timelapseName)) {}
   
   std::string myTimelapseName{"None"};
};

struct CmdUserMove : Command
{
   CmdUserMove() : Command(CommandTypeEnum::USER_MOVE) {}
   CmdUserMove(const double& thetaInDeg, const double& phiInDeg) :
      Command(CommandTypeEnum::USER_MOVE),
      myThetaInDeg(thetaInDeg),
      myPhiInDeg(phiInDeg) {}

   double myThetaInDeg{0.0};
   double myPhiInDeg{0.0};
};

struct CmdUserFocus : Command
{
   CmdUserFocus() : Command(CommandTypeEnum::USER_FOCUS) {}
   explicit CmdUserFocus(const double& thetaInDeg) :
      Command(CommandTypeEnum::USER_FOCUS),
      myThetaInDeg(thetaInDeg) {}

   double myThetaInDeg{0.0};
};

struct CmdFollowTarget : Command
{
   CmdFollowTarget() : Command(CommandTypeEnum::FOLLOW_TARGET), myStartTime(std::chrono::system_clock::now()) {}
   CmdFollowTarget(std::string targetName, const std::chrono::seconds& duration) : 
      Command(CommandTypeEnum::FOLLOW_TARGET),
      myTargetName(std::move(targetName)),
      myDuration(duration),
      myStartTime(std::chrono::system_clock::now()) {}

   std::string myTargetName{"None"};
   std::chrono::seconds myDuration{0};
   std::chrono::time_point<std::chrono::system_clock> myStartTime;
};

struct CmdGoToTarget : Command
{
   CmdGoToTarget() : Command(CommandTypeEnum::GOTO_TARGET) {}
   explicit CmdGoToTarget(std::string targetName) : 
      Command(CommandTypeEnum::GOTO_TARGET),
      myTargetName(std::move(targetName)) {}

   std::string myTargetName{"None"};
};

struct CmdSearchTarget : Command
{
   CmdSearchTarget() : Command(CommandTypeEnum::SEARCH_TARGET) {}
   CmdSearchTarget(std::string targetName, const double& searchRadiusInLightYears) :
      Command(CommandTypeEnum::SEARCH_TARGET),
      myTargetName(std::move(targetName)),
      mySearchRadiusInLightYears(searchRadiusInLightYears) {}

   std::string myTargetName{"None"};
   double mySearchRadiusInLightYears{0.0};
};

struct CmdCalibrate : Command
{
   CmdCalibrate() : Command(CommandTypeEnum::CALIBRATE) {}
};

#endif