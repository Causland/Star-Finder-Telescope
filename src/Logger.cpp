#include "Logger.hpp"

#include <iomanip>
#include <sstream>

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::shared_ptr<std::ostream> Logger::theOutputStream{nullptr};
std::string Logger::theLogToWrite{};
bool Logger::theLogsAvailableFlag{false};
bool Logger::theExitingFlag{false};
std::condition_variable Logger::theCondVar{};
std::mutex Logger::theMutex{};
std::thread Logger::theThread{};
std::queue<std::string> Logger::theLogsToRecord{};
bool Logger::theInitializedFlag{false};
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

void Logger::initialize(std::shared_ptr<std::ostream> stream)
{
   if (!theInitializedFlag) // Only allow initialization once unless terminated
   {
      theInitializedFlag = true;
      theOutputStream = std::move(stream);
      theThread = std::thread(Logger::threadLoop);
   }
}

void Logger::terminate()
{
   theExitingFlag = true;
   theCondVar.notify_one();
   theThread.join();

   {
      std::scoped_lock<std::mutex> lock(theMutex);
      if (theLogsAvailableFlag) 
      {
         processLogs();
      }
   }

   writeToLog();
   theOutputStream = nullptr;
   theInitializedFlag = false;
   theExitingFlag = false;
}

void Logger::log(std::string_view fileName, const uint32_t& lineNum,
                 const LogCodeEnum code, std::string_view msg)
{
   {
      std::scoped_lock<std::mutex> lock(theMutex);
      theLogsToRecord.push(logToString(fileName, lineNum, code, msg, std::chrono::system_clock::now()));
      theLogsAvailableFlag = true;
   }
   // Signal to the condition variable to wake up the logging thread
   theCondVar.notify_one();
}

std::string Logger::logToString(const std::string_view fileName, const uint32_t& lineNum,
                                const LogCodeEnum code, const std::string_view msg,
                                const std::chrono::time_point<std::chrono::system_clock>& timePoint)
{
   static constexpr uint16_t CODE_WIDTH{5};

   // Logs are in format: DATETIME | [LOG_CODE] MESSAGE (FILE:LINE)
   const auto time{std::chrono::system_clock::to_time_t(timePoint)};
   std::ostringstream oss;
   oss << std::put_time(std::localtime(&time), "%T") 
       << " | "
       << "[" << std::setw(CODE_WIDTH) << logCodeToString(code) << "]"
       << " "
       << msg
       << " "
       << "(" << fileName << ":" << lineNum << ")"
       << "\n";
   return oss.str();
}

void Logger::threadLoop()
{
   while (!theExitingFlag)
   {
      // Wait until log is written into the queue
      std::unique_lock<std::mutex> lock(theMutex);
      theCondVar.wait(lock, [&]{ return theLogsAvailableFlag || theExitingFlag; });
      if (theExitingFlag) // If signaled by the destructor, need to exit immediately
      {
         break;
      }

      // Access logs queue and append new information to output string
      processLogs();
      
      // Release the mutex as soon as possible to prevent delays in other threads
      lock.unlock();

      // Write the logs to the output file
      writeToLog();
   }
}

void Logger::processLogs()
{
   while (!theLogsToRecord.empty())
   {
      // Append each log to the output string for writing to log file
      theLogToWrite += theLogsToRecord.front();
      theLogsToRecord.pop();
   }
   theLogsAvailableFlag = false;
}

void Logger::writeToLog()
{
   if (!theLogToWrite.empty())
   {
      // Write to output stream and flush to immediately sync
      if (theOutputStream != nullptr)
      {
         *theOutputStream << theLogToWrite;
         theOutputStream->flush();
      }
   }
   theLogToWrite.clear();
}

