#include "gtest/gtest.h"
#include "CT_OpticsManager.hpp"
#include "CT_PositionManager.hpp"
#include "CT_StarTracker.hpp"
#include "CommandTerminal.hpp"
#include "MinInformationDisplay.hpp"
#include <iostream>

class TestFixtureCommandTerminal : public ::testing::Test
{
public:
   void SetUp() override
   {
      // Create all required CommandTerminal objects
      logger = std::make_shared<Logger>("logs/CommandTerminalUnitTest.log");
      exitSignal = std::make_shared<std::atomic<bool>>(false);
      subsystems.push_back(std::make_shared<ISubsystem>(CT_OpticsManager()));
      subsystems.push_back(std::make_shared<ISubsystem>(CT_PositionManager()));
      subsystems.push_back(std::make_shared<ISubsystem>(CT_StarTracker()));
      subsystems.push_back(std::make_shared<ISubsystem>(MinInformationDisplay()));

      // Create the test subsystem
      commandTerminal = std::make_unique<CommandTerminal>("CommandTerminal", logger, exitSignal);
      commandTerminal->configureInterfaces(subsystems);
      commandTerminal->start();
   }
   void TearDown() override
   {
      commandTerminal->stop();
   }

   std::shared_ptr<Logger> logger{nullptr};
   std::shared_ptr<std::atomic<bool>> exitSignal{nullptr};
   std::vector<std::shared_ptr<ISubsystem>> subsystems{};
   std::unique_ptr<CommandTerminal> commandTerminal{nullptr};
};

TEST_F(TestFixtureCommandTerminal, CommandProcessing)
{
   // Redirect std::cin to look for test buffer instead of normal cin
   auto originalBuffer = std::cin.rdbuf();
   std::stringstream inputStream;
   std::cin.rdbuf(inputStream.rdbuf());

   // Photo command checks
   {
      std::string name{"TestPhoto.jpg"};
      // Valid Format
      std::string testCommand{"photo " + name};
      ASSERT_EQ(true, commandTerminal->interpretCommand(testCommand));
   }
}