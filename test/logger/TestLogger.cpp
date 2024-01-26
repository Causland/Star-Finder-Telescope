#include "gtest/gtest.h"
#include "Logger.hpp"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

namespace detail
{
   // Used to create string vector from stream line by line
   class Line : public std::string
   {
      friend std::istream& operator>>(std::istream& stream, Line& line)
      {
	 return std::getline(stream, line);
      }
   };
} // namespace detail

using InIt = std::istream_iterator<detail::Line>;

class TestFixtureLogger : public ::testing::Test
{
protected:
   static Logger logger; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
};

Logger TestFixtureLogger::logger{}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)


TEST_F(TestFixtureLogger, InitTerm)
{
   // Initialize and terminate the logger multiple times and verify
   // the log file was created successfully
   
   const auto initSleepTerm{[]()
			   {
			      auto oss{std::make_shared<std::ostringstream>()};
                              Logger::initialize(oss);
                              LOG_INFO("Testing logging");
			      Logger::terminate();
			      EXPECT_TRUE(oss->str().find("Testing logging") != std::string::npos);
		           }};

   static constexpr uint16_t NUM_ATTEMPTS{5};

   for (auto i=0; i<NUM_ATTEMPTS; ++i)
   {
      initSleepTerm();
   }
}

TEST_F(TestFixtureLogger, Macros)
{
   // Attempt to write to the log using each LOG_ macro
   auto stream{std::make_shared<std::stringstream>()};
   Logger::initialize(stream);

   LOG_INFO("info");
   LOG_WARN("warning");
   LOG_ERROR("error");
   LOG_DEBUG("debug");

   Logger::terminate();

   std::vector<std::string> lines(InIt{*stream}, InIt{});
   
   ASSERT_EQ(lines.size(), 4);
   // NOLINTBEGIN(readability-identifier-length)
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("INFO") != std::string::npos; }), 1);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("WARN") != std::string::npos; }), 1);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("ERROR") != std::string::npos; }), 1);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("DEBUG") != std::string::npos; }), 1);
   // NOLINTEND(readability-identifier-length)
}

TEST_F(TestFixtureLogger, HighDemand)
{
   // Use multiple threads to log at periodic fast rates
   
   const auto logRepeatedly{[](const LogCodeEnum code, const uint16_t count, const std::chrono::milliseconds& interval)
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
			      }};

   auto stream{std::make_shared<std::stringstream>()};
   Logger::initialize(stream);

   // NOLINTBEGIN(readability-identifier-length, readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
   std::thread t1{std::thread(logRepeatedly, LogCodeEnum::ERROR, 2000, std::chrono::milliseconds(1))};
   std::thread t2{std::thread(logRepeatedly, LogCodeEnum::WARNING, 1000, std::chrono::milliseconds(2))};
   std::thread t3{std::thread(logRepeatedly, LogCodeEnum::INFO, 500, std::chrono::milliseconds(4))};
   std::thread t4{std::thread(logRepeatedly, LogCodeEnum::DEBUG, 250, std::chrono::milliseconds(8))};
   // NOLINTEND(readability-identifier-length, readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)

   t1.join();
   t2.join();
   t3.join();
   t4.join();

   Logger::terminate();

   std::vector<std::string> lines(InIt{*stream}, InIt{});

   // NOLINTBEGIN(readability-identifier-length)
   ASSERT_EQ(lines.size(), 3750);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("ERROR") != std::string::npos; }), 2000);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("WARN") != std::string::npos; }), 1000);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("INFO") != std::string::npos; }), 500);
   EXPECT_EQ(std::count_if(std::begin(lines), std::end(lines), 
			      [](const auto& s){ return s.find("DEBUG") != std::string::npos; }), 250);
   // NOLINTEND(readability-identifier-length)
}
