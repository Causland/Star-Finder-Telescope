#include "gtest/gtest.h"
#include "CT_OpticsManager.hpp"
#include "CT_PositionManager.hpp"
#include "CT_StarTracker.hpp"
#include "CommandTerminal.hpp"
#include "MinInformationDisplay.hpp"
#include <chrono>
#include <iostream>
#include <thread>

class TestFixtureCommandTerminal : public ::testing::Test
{
protected:
   static void SetUpTestSuite()
   {
      // Create all required CommandTerminal objects
      logger = std::make_shared<Logger>("logs/CommandTerminalUnitTest.log");
      exitSignal = std::make_shared<std::atomic<bool>>(false);
      opticsManager = std::make_shared<CT_OpticsManager>(CT_OpticsManager());
      starTracker = std::make_shared<CT_StarTracker>(CT_StarTracker());
      positionManager = std::make_shared<CT_PositionManager>(CT_PositionManager());
      informationDisplay = std::make_shared<MinInformationDisplay>(MinInformationDisplay());
      subsystems.emplace_back(opticsManager);
      subsystems.emplace_back(starTracker);
      subsystems.emplace_back(positionManager);
      subsystems.emplace_back(informationDisplay);

      // Create the test subsystem
      commandTerminal = std::make_unique<CommandTerminal>("CommandTerminal", logger, exitSignal);
      commandTerminal->configureInterfaces(subsystems);
   }

   bool checkReceived()
   {
      return (opticsManager->myCommandReceived || positionManager->myCommandReceived || starTracker->myCommandReceived);
   }

   void beginSubTest(std::stringstream& ss)
   {
      // Change std::cin to look for the test input stream
      std::cin.rdbuf(ss.rdbuf());

      // Start command processing and check results
      commandTerminal->start();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
   }

   void endSubTest()
   {
      commandTerminal->stop();
      opticsManager->reset();
      positionManager->reset();
      starTracker->reset();
   }

   static std::shared_ptr<Logger> logger;
   static std::shared_ptr<std::atomic<bool>> exitSignal;
   static std::vector<std::shared_ptr<ISubsystem>> subsystems;
   static std::shared_ptr<CT_OpticsManager> opticsManager;
   static std::shared_ptr<CT_PositionManager> positionManager;
   static std::shared_ptr<CT_StarTracker> starTracker;
   static std::shared_ptr<MinInformationDisplay> informationDisplay;
   static std::unique_ptr<CommandTerminal> commandTerminal;
};

// Define static member variables from test fixture
std::shared_ptr<Logger> TestFixtureCommandTerminal::logger{};
std::shared_ptr<std::atomic<bool>> TestFixtureCommandTerminal::exitSignal{};
std::vector<std::shared_ptr<ISubsystem>> TestFixtureCommandTerminal::subsystems{};
std::shared_ptr<CT_OpticsManager> TestFixtureCommandTerminal::opticsManager{};
std::shared_ptr<CT_PositionManager> TestFixtureCommandTerminal::positionManager{};
std::shared_ptr<CT_StarTracker> TestFixtureCommandTerminal::starTracker{};
std::shared_ptr<MinInformationDisplay> TestFixtureCommandTerminal::informationDisplay{};
std::unique_ptr<CommandTerminal> TestFixtureCommandTerminal::commandTerminal{};

TEST_F(TestFixtureCommandTerminal, CommandProcessing_PhotoCommand)
{
   // Valid command
   {
      const std::string name{"TestPhoto"};
      std::stringstream testCommand{"photo " + name};

      beginSubTest(testCommand);
      ASSERT_EQ(name, opticsManager->myTakePhotoCmd.myPhotoName);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"photo"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"photo name extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_VideoCommand)
{
   // Valid command
   {
      const std::string name{"TestVideo"};
      const uint64_t duration{5};
      std::stringstream testCommand{"video " + name + " " + std::to_string(duration)};

      beginSubTest(testCommand);
      ASSERT_EQ(name, opticsManager->myTakeVideoCmd.myVideoName);
      ASSERT_EQ(std::chrono::seconds(duration), opticsManager->myTakeVideoCmd.myDuration);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"video"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"video name"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad int conversion
   {
      std::stringstream testCommand{"video name invalid"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"video name 5 extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_TimelapseCommand)
{
   // Valid command
   {
      const std::string name{"TestTimelapse"};
      const uint64_t duration{5};
      const double rate{10.3};
      std::stringstream testCommand{"timelapse " + name + " " + std::to_string(duration) + " " + std::to_string(rate)};

      beginSubTest(testCommand);
      ASSERT_EQ(name, opticsManager->myTakeTimelapseCmd.myTimelapseName);
      ASSERT_EQ(std::chrono::minutes(duration), opticsManager->myTakeTimelapseCmd.myDuration);
      ASSERT_EQ(rate, opticsManager->myTakeTimelapseCmd.myRateInHz);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"timelapse"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"timelapse name"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"timelapse name 2"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad int conversion
   {
      std::stringstream testCommand{"timelapse name 5.5 2.2"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad double conversion
   {
      std::stringstream testCommand{"timelapse name 6 invalid"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"timelapse name 22 7.6 extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_MoveCommand)
{
   // Valid command
   {
      const double theta{6.8};
      const double phi{32.7};
      std::stringstream testCommand{"move " + std::to_string(theta) + " " + std::to_string(phi)};

      beginSubTest(testCommand);
      ASSERT_EQ(theta, positionManager->myUserMoveCmd.myThetaInDeg);
      ASSERT_EQ(phi, positionManager->myUserMoveCmd.myPhiInDeg);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"move"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"move 5.3"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad double conversion
   {
      std::stringstream testCommand{"move invalid 2.2"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad double conversion
   {
      std::stringstream testCommand{"move 5.7 invalid"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"move 7.4 3.8 extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_FocusCommand)
{
   // Valid command
   {
      const double theta{56.9};
      std::stringstream testCommand{"focus " + std::to_string(theta)};

      beginSubTest(testCommand);
      ASSERT_EQ(theta, opticsManager->myUserFocusCmd.myThetaInDeg);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"focus"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad double conversion
   {
      std::stringstream testCommand{"focus invalid"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"focus 33.5 extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_FollowCommand)
{
   // Valid command
   {
      const std::string name{"Jupiter"};
      const uint64_t duration{76};
      std::stringstream testCommand{"follow " + name + " " + std::to_string(duration)};

      beginSubTest(testCommand);
      ASSERT_EQ(name, starTracker->myFollowTargetCmd.myTargetName);
      ASSERT_EQ(std::chrono::seconds(duration), starTracker->myFollowTargetCmd.myDuration);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"follow"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"follow name"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad int conversion
   {
      std::stringstream testCommand{"follow name 6.4"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"follow name 9 extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_GoToCommand)
{
   // Valid command
   {
      const std::string name{"Pluto"};
      std::stringstream testCommand{"goto " + name};

      beginSubTest(testCommand);
      ASSERT_EQ(name, starTracker->myGoToTargetCmd.myTargetName);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"goto"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"goto name extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_SearchRangeCommand)
{
   // Valid command
   {
      const double range{5233.74};
      std::stringstream testCommand{"search range " + std::to_string(range)};

      beginSubTest(testCommand);
      ASSERT_EQ(range, starTracker->mySearchTargetCmd.mySearchRadiusInLightYears);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"search range"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad double conversion
   {
      std::stringstream testCommand{"search range invalid"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }   
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"search range 4.7 extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_SearchNameCommand)
{
   // Valid command
   {
      const std::string name{"Mercu*"};
      std::stringstream testCommand{"search name " + name};

      beginSubTest(testCommand);
      ASSERT_EQ(name, starTracker->mySearchTargetCmd.myTargetName);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"search name"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> extra param
   {
      std::stringstream testCommand{"search name planet extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_CalibrateCommand)
{
   // Valid command
   {
      std::stringstream testCommand{"calibrate"};

      beginSubTest(testCommand);
      ASSERT_EQ(true, positionManager->myCommandReceived);
      endSubTest();
   }
      // Invalid command -> extra param
   {
      std::stringstream testCommand{"calibrate extra"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessing_Misc)
{

   // Invalid command -> unknown base command
   {
      std::stringstream testCommand{"unknown"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> unknown search secondary command
   {
      std::stringstream testCommand{"search unknown"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}