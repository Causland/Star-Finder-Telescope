#include "interfaces/IOpticsManager.hpp"
#include "interfaces/ISubsystem.hpp"

const std::string IOpticsManager::NAME{"OpticsManager"};

class MinOpticsManager : public IOpticsManager, public ISubsystem
{
public:
   std::string takePhoto(const CmdTakePhoto& cmd) override { return ""; }
   std::string takeVideo(const CmdTakeVideo& cmd) override { return ""; } 
   std::string takeTimelapse(const CmdTakeTimelapse& cmd) override { return ""; }
   void userChangeFocus(const CmdUserFocus& cmd) {}
   void start() override {}
   void stop() override {}
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};