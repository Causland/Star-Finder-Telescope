#include "interfaces/IInformationDisplay.hpp"
#include "interfaces/ISubsystem.hpp"

const std::string IInformationDisplay::NAME{"InformationDisplay"};

class MinInformationDisplay : public IInformationDisplay, public ISubsystem
{
public:
   void start() override {}
   void stop() override {}
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};