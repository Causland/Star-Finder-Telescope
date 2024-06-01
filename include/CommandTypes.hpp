#ifndef COMMAND_TYPES_HPP
#define COMMAND_TYPES_HPP

#include "Common.hpp"

#include <chrono>
#include <string>
#include <string_view>

enum class CommandTypeEnum
{
   TAKE_PHOTO,
   TAKE_VIDEO,
   TAKE_TIMELAPSE,
   UPDATE_POSITION,
   USER_FOCUS,
   FOLLOW_TARGET,
   GOTO_TARGET,
   SEARCH_TARGET,
   CALIBRATE,
};

static constexpr std::string_view commandTypeToString(const CommandTypeEnum cmdType)
{
   switch(cmdType)
   {
      case CommandTypeEnum::TAKE_PHOTO: return "PHOTO";
      case CommandTypeEnum::TAKE_VIDEO: return "VIDEO";
      case CommandTypeEnum::TAKE_TIMELAPSE: return "TIMELAPSE";
      case CommandTypeEnum::UPDATE_POSITION: return "MOVE";
      case CommandTypeEnum::USER_FOCUS: return "FOCUS";
      case CommandTypeEnum::FOLLOW_TARGET: return "FOLLOW";
      case CommandTypeEnum::GOTO_TARGET: return "GOTO";
      case CommandTypeEnum::SEARCH_TARGET: return "SEARCH";
      case CommandTypeEnum::CALIBRATE: return "CALIBRATE";
   }
   return "UNKNOWN";
}

struct Command
{
   explicit Command(const CommandTypeEnum commandType) : myCommandType{commandType} {}

   CommandTypeEnum myCommandType;
};

struct CmdTakePhoto : public Command
{
   CmdTakePhoto() : Command{CommandTypeEnum::TAKE_PHOTO} {} 
   explicit CmdTakePhoto(std::string photoName) : 
      Command{CommandTypeEnum::TAKE_PHOTO}, 
      myPhotoName{std::move(photoName)} {}
   
   std::string myPhotoName{"None"};
};

struct CmdTakeVideo : public Command
{
   CmdTakeVideo() : Command{CommandTypeEnum::TAKE_VIDEO} {}
   explicit CmdTakeVideo(std::string videoName, const std::chrono::seconds& duration) : 
      Command{CommandTypeEnum::TAKE_VIDEO},
      myVideoName{std::move(videoName)},
      myDuration{duration},
      myStartTime{std::chrono::system_clock::now()} {}

   std::string myVideoName{"None"};
   std::chrono::seconds myDuration{0};
   std::chrono::time_point<std::chrono::system_clock> myStartTime;
};

struct CmdTakeTimelapse : public Command
{
   CmdTakeTimelapse() : Command{CommandTypeEnum::TAKE_TIMELAPSE} {}
   explicit CmdTakeTimelapse(std::string timelapseName, const std::chrono::minutes& duration, const double rate) : 
      Command{CommandTypeEnum::TAKE_TIMELAPSE},
      myTimelapseName{std::move(timelapseName)},
      myDuration{duration},
      myRateInHz{rate},
      myStartTime{std::chrono::system_clock::now()} {}
   
   std::string myTimelapseName{"None"};
   std::chrono::minutes myDuration{0};
   double myRateInHz{0.0};
   std::chrono::time_point<std::chrono::system_clock> myStartTime;
};

struct CmdUpdatePosition : public Command
{
   CmdUpdatePosition() : Command{CommandTypeEnum::UPDATE_POSITION} {}
   explicit CmdUpdatePosition(const Position& pos) :
      Command{CommandTypeEnum::UPDATE_POSITION},
      myPosition{pos} {}

   Position myPosition{0.0, 0.0};
};

struct CmdUserFocus : public Command
{
   CmdUserFocus() : Command{CommandTypeEnum::USER_FOCUS} {}
   explicit CmdUserFocus(const double& thetaInDeg) :
      Command{CommandTypeEnum::USER_FOCUS},
      myThetaInDeg{thetaInDeg} {}

   double myThetaInDeg{0.0};
};

struct CmdFollowTarget : public Command
{
   CmdFollowTarget() : Command{CommandTypeEnum::FOLLOW_TARGET}, myStartTime{std::chrono::system_clock::now()} {}
   CmdFollowTarget(std::string targetName, const std::chrono::seconds& duration) : 
      Command{CommandTypeEnum::FOLLOW_TARGET},
      myTargetName{std::move(targetName)},
      myDuration{duration},
      myStartTime{std::chrono::system_clock::now()} {}

   std::string myTargetName{"None"};
   std::chrono::seconds myDuration{0};
   std::chrono::time_point<std::chrono::system_clock> myStartTime;
};

struct CmdGoToTarget : public Command
{
   CmdGoToTarget() : Command{CommandTypeEnum::GOTO_TARGET} {}
   explicit CmdGoToTarget(std::string targetName) : 
      Command{CommandTypeEnum::GOTO_TARGET},
      myTargetName{std::move(targetName)} {}

   std::string myTargetName{"None"};
};

struct SearchTargetParams
{
   std::string myTargetName;
   double mySearchRadiusInLightYears{0.0};
   double mySearchLuminosityInWatts{0.0};
};

struct CmdSearchTarget : public Command
{
   CmdSearchTarget() : Command{CommandTypeEnum::SEARCH_TARGET} {}
   explicit CmdSearchTarget(const SearchTargetParams& params) :
      Command{CommandTypeEnum::SEARCH_TARGET},
      myTargetName{params.myTargetName},
      mySearchRadiusInLightYears{params.mySearchRadiusInLightYears},
      mySearchLuminosityInWatts{params.mySearchLuminosityInWatts} {}

   std::string myTargetName{"None"};
   double mySearchRadiusInLightYears{-1.0};
   double mySearchLuminosityInWatts{-1.0};
};

struct CmdCalibrate : public Command
{
   CmdCalibrate() : Command{CommandTypeEnum::CALIBRATE} {}
};

#endif
