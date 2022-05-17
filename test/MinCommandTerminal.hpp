#include "interfaces/ICommandTerminal.hpp"
#include "interfaces/ISubsystem.hpp"

class MinCommandTerminal : public ICommandTerminal, public ISubsystem
{
public:
   void start() override {}
   void stop() override {}
   void configureInterfaces(const std::vector<std::shared_ptr<ISubsystem>>& subsystems) override {}
   bool checkHeartbeat() { return true; }
   std::string getName() { return NAME; }
};