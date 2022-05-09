#include "gtest/gtest.h"
#include "CommandTerminal.hpp"

class TestFixtureCommandTerminal : public ::testing::Test
{
protected:
   void SetUp() override
   {
      logger = std::make_shared<Logger>("logs/CommandTerminalUnitTest.log");
      exitSignal = std::make_shared<std::atomic<bool>>(false);
      commandTerminal = std::make_unique<CommandTerminal>("CommandTerminal", logger, exitSignal);
      commandTerminal->start();
   }
   void TearDown() override
   {
      commandTerminal->stop();
   }

   std::shared_ptr<Logger> logger{nullptr};
   std::shared_ptr<std::atomic<bool>> exitSignal{nullptr};
   std::unique_ptr<CommandTerminal> commandTerminal{nullptr};
};

TEST_F(TestFixtureCommandTerminal, CinCommandParsing)
{
   ASSERT_EQ(commandTerminal->getName(), "CommandTerminal");
}