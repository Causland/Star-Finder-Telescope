#include "interfaces/IOpticsManager.hpp"

const std::string IOpticsManager::NAME{"SimOpticsManager"};

class MinOpticsManager : public IOpticsManager
{
public:
   std::string takePhoto(const CmdTakePhoto& cmd) override { return ""; }
   std::string takeVideo(const CmdTakeVideo& cmd) override { return ""; } 
   std::string takeTimelapse(const CmdTakeTimelapse& cmd) override { return ""; }
   void userChangeFocus(const CmdUserFocus& cmd) {}
};