#include "CommandTerminal.hpp"

class MinCommandTerminal : public CommandTerminal
{
public:
   MinCommandTerminal() : CommandTerminal(NAME, nullptr) {};
   void start() override {}
   void stop() override {}
   void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override {}
   bool checkHeartbeat() override { return true; }
   std::string getName() override { return NAME; }
};