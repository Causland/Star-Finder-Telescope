#include "gtest/gtest.h"
#include "Logger.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

namespace detail
{
   // Used to create string vector from file line by line
   class Line : public std::string
   {
      friend std::istream& operator>>(std::istream& is, Line& line)
      {
	 return std::getline(is, line);
      }
   };
}
typedef std::istream_iterator<detail::Line> InIt;


class TestFixtureLogger : public ::testing::Test
{
protected:
   static void SetUpTestSuite()
   {
      // Create folder for test files
      std::srand(static_cast<unsigned int>(std::time(nullptr)));
      testoutDir = testoutDir + std::to_string(rand()) + "/";
      std::filesystem::remove_all(testoutDir); // Remove on small chance of duplicate
      std::filesystem::create_directory(testoutDir);
   }
   static void TearDownTestSuite()
   {
      // Delete any file artifacts from testing
      std::filesystem::remove_all(testoutDir);
   }

   static std::string testoutDir;
   static Logger logger;
};

std::string TestFixtureLogger::testoutDir{"logger_testout_"};
Logger TestFixtureLogger::logger{};


TEST_F(TestFixtureLogger, Init_Term)
{
   // Initialize and terminate the logger multiple times and verify
   // the log file was created successfully
   
   const auto initSleepTerm = [](const std::string& logName)
			   {
                              Logger::initialize(logName);
			      std::this_thread::sleep_for(std::chrono::milliseconds(10));
                              LOG_INFO("Creating " + logName);
			      std::this_thread::sleep_for(std::chrono::milliseconds(100));
			      Logger::terminate();
			      EXPECT_TRUE(std::filesystem::exists(logName));
		           };

   for (auto i=0; i<10; ++i)
   {
      initSleepTerm(testoutDir + std::to_string(i) + ".log");
   }
}

TEST_F(TestFixtureLogger, Macros)
{
   // Attempt to write to the log using each LOG_ macro

   const std::string macroLog{testoutDir + std::string("macros.log")};
   Logger::initialize(macroLog);
   std::this_thread::sleep_for(std::chrono::milliseconds(10));

   LOG_INFO("info");
   LOG_WARN("warning");
   LOG_ERROR("error");
   LOG_DEBUG("debug");

   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   std::ifstream is{macroLog};
   ASSERT_TRUE(is.is_open());
   std::vector<std::string> lines(InIt{is}, InIt{});
   
   ASSERT_EQ(lines.size(), 4);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("INFO") != std::string::npos; }), 1);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("WARN") != std::string::npos; }), 1);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("ERROR") != std::string::npos; }), 1);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("DEBUG") != std::string::npos; }), 1);

   Logger::terminate();
}

TEST_F(TestFixtureLogger, HighDemand)
{
   // Use multiple threads to log at periodic fast rates
   
   const auto logRepeatedly = [](const LogCodeEnum code, const uint16_t count, const std::chrono::milliseconds& interval)
			      {
				 for (auto i=0; i<count; ++i)
				 {
				    switch(code)
				    {
				       case LogCodeEnum::ERROR: LOG_ERROR("error"); break;
				       case LogCodeEnum::WARNING: LOG_WARN("warning"); break;
				       case LogCodeEnum::INFO: LOG_INFO("info"); break;
				       case LogCodeEnum::DEBUG: LOG_DEBUG("debug"); break;
				    }
				    std::this_thread::sleep_for(interval);
				 }
			      };

   const std::string demandLog{testoutDir + std::string("high_demand.log")};
   Logger::initialize(demandLog);
   std::this_thread::sleep_for(std::chrono::milliseconds(10));

   std::thread t1{std::thread(logRepeatedly, LogCodeEnum::ERROR, 1000, std::chrono::milliseconds(2))};
   std::thread t2{std::thread(logRepeatedly, LogCodeEnum::WARNING, 500, std::chrono::milliseconds(4))};
   std::thread t3{std::thread(logRepeatedly, LogCodeEnum::INFO, 250, std::chrono::milliseconds(8))};
   std::thread t4{std::thread(logRepeatedly, LogCodeEnum::DEBUG, 125, std::chrono::milliseconds(16))};

   t1.join();
   t2.join();
   t3.join();
   t4.join();

   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   std::ifstream is{demandLog};
   ASSERT_TRUE(is.is_open());
   std::vector<std::string> lines(InIt{is}, InIt{});

   ASSERT_EQ(lines.size(), 1875);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("ERROR") != std::string::npos; }), 1000);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("WARN") != std::string::npos; }), 500);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("INFO") != std::string::npos; }), 250);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("DEBUG") != std::string::npos; }), 125);

   Logger::terminate();
}
