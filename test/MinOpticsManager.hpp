#include "OpticsManager.hpp"

class MinOpticsManager : public OpticsManager
{
public:
   MinOpticsManager() : OpticsManager(NAME) {}
   std::string takePhoto(const CmdTakePhoto& cmd) override { return ""; }
   std::string takeVideo(const CmdTakeVideo& cmd) override { return ""; } 
   std::string takeTimelapse(const CmdTakeTimelapse& cmd) override { return ""; }
   void userChangeFocus(const CmdUserFocus& cmd) {}
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};