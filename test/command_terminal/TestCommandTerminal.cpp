#include "gtest/gtest.h"
#include "CT_OpticsManager.hpp"
#include "CT_PositionManager.hpp"
#include "CT_StarTracker.hpp"
#include "CommandTerminal.hpp"
#include "Logger.hpp"
#include "MinInformationDisplay.hpp"
#include <chrono>
#include <iostream>
#include <thread>

static constexpr uint16_t WAIT_FOR_PROCESSING_MS{50};

class TestFixtureCommandTerminal : public ::testing::Test
{
protected:
   TestFixtureCommandTerminal() :
         exitSignal{std::make_shared<std::atomic<bool>>(false)},
         opticsManager{std::make_shared<CT_OpticsManager>()},
         positionManager{std::make_shared<CT_PositionManager>()},
         starTracker{std::make_shared<CT_StarTracker>()},
         informationDisplay{std::make_shared<MinInformationDisplay>()},
         commandTerminal{std::make_unique<CommandTerminal>(exitSignal)}
   {
      // Create the subsystems vector
      subsystems[static_cast<int>(SubsystemEnum::OPTICS_MANAGER)] = opticsManager;
      subsystems[static_cast<int>(SubsystemEnum::STAR_TRACKER)] = starTracker;
      subsystems[static_cast<int>(SubsystemEnum::POSITION_MANAGER)] = positionManager;
      subsystems[static_cast<int>(SubsystemEnum::INFORMATION_DISPLAY)] = informationDisplay;

      // Initialize the Command Terminal subsystem
      commandTerminal->configureSubsystems(subsystems);
   }

   static void SetUpTestSuite()
   {
      Logger::initialize(std::make_shared<std::ofstream>("logs/CommandTerminalUnitTest.log"));
   }

   static void TearDownTestSuite()
   {
      Logger::terminate();
   }

   bool checkReceived()
   {
      return (opticsManager->myCommandReceived || positionManager->myCommandReceived || starTracker->myCommandReceived);
   }

   void beginSubTest(std::stringstream& stream)
   {
      // Change std::cin to look for the test input stream
      std::cin.rdbuf(stream.rdbuf());

      // Start command processing and check results
      commandTerminal->start();
      std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_PROCESSING_MS));
   }

   void endSubTest()
   {
      commandTerminal->stop();
      opticsManager->reset();
      positionManager->reset();
      starTracker->reset();
   }

   
   static Logger logger; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
   std::shared_ptr<std::atomic<bool>> exitSignal;
   std::array<std::shared_ptr<Subsystem>, static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)> subsystems;
   std::shared_ptr<CT_OpticsManager> opticsManager;
   std::shared_ptr<CT_PositionManager> positionManager;
   std::shared_ptr<CT_StarTracker> starTracker;
   std::shared_ptr<MinInformationDisplay> informationDisplay;
   std::unique_ptr<CommandTerminal> commandTerminal;
};

Logger TestFixtureCommandTerminal::logger{}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)


TEST_F(TestFixtureCommandTerminal, CommandProcessingPhotoCommand)
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingVideoCommand)
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingTimelapseCommand)
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
   // Invalid command -> bad double conversion
   {
      std::stringstream testCommand{"timelapse name 6 invalid"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingMoveCommand)
{
   // Valid command
   {
      const double theta{6.8};
      const double phi{32.7};
      std::stringstream testCommand{"move " + std::to_string(theta) + " " + std::to_string(phi)};

      beginSubTest(testCommand);
      ASSERT_EQ(theta, positionManager->myUpdatePositionCmd.myPosition.myAzimuth);
      ASSERT_EQ(phi, positionManager->myUpdatePositionCmd.myPosition.myElevation);
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingFocusCommand)
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingFollowCommand)
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingGoToCommand)
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingSearchRangeCommand)
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingSearchBrightnessCommand)
{
   // Valid command
   {
      const double brightness{5032.64};
      std::stringstream testCommand{"search brightness " + std::to_string(brightness)};

      beginSubTest(testCommand);
      ASSERT_EQ(brightness, starTracker->mySearchTargetCmd.mySearchLuminosityInWatts);
      endSubTest();
   }
   // Invalid command -> missing param
   {
      std::stringstream testCommand{"search brightness"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }
   // Invalid command -> bad double conversion
   {
      std::stringstream testCommand{"search brightness invalid"};

      beginSubTest(testCommand);
      ASSERT_EQ(false, checkReceived());
      endSubTest();
   }   
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingSearchNameCommand)
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
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingCalibrateCommand)
{
   // Valid command
   {
      std::stringstream testCommand{"calibrate"};

      beginSubTest(testCommand);
      ASSERT_EQ(true, positionManager->myCommandReceived);
      endSubTest();
   }
}

TEST_F(TestFixtureCommandTerminal, CommandProcessingMisc)
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
