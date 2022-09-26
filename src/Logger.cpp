#include "Logger.hpp"
#include <iostream>

std::string Logger::theFileName{};
std::ofstream Logger::theOutputFile{};
std::string Logger::theLogToWrite{};
bool Logger::theLogsAvailableFlag{false};
bool Logger::theExitingFlag{false};
std::condition_variable Logger::theCondVar{};
std::mutex Logger::theMutex{};
std::thread Logger::theThread{};
std::queue<std::string> Logger::theLogsToRecord{};
bool Logger::theInitializedFlag{false};

void Logger::initialize(const std::string& fileName)
{
   if (!theInitializedFlag) // Only allow initialization once unless terminated
   {
      theInitializedFlag = true;
      theFileName = fileName;
      theThread = std::thread(Logger::threadLoop);
   }
}

void Logger::terminate()
{
   theExitingFlag = true;
   theLogsAvailableFlag = true;
   theCondVar.notify_one();
   theThread.join();
   processLogs();
   theInitializedFlag = false;
}

void Logger::log(const std::string& subsystemName, const LogCodeEnum& code, const std::string& message)
{
   // Create the log message and get the string representation
   LogMessage messageToLog(subsystemName, code, message);
   std::string logString = messageToLog.toString();
   {
      std::scoped_lock<std::mutex> lk(theMutex);
      theLogsToRecord.push(logString);
      theLogsAvailableFlag = true;
   }
   // Signal to the condition variable to wake up the logging thread
   theCondVar.notify_one();
}

void Logger::threadLoop()
{
   while (!theExitingFlag)
   {
      // Wait until log is written into the queue
      std::unique_lock<std::mutex> lk(theMutex);
      theCondVar.wait(lk, [&]{ return theLogsAvailableFlag; });
      if (theExitingFlag) // If signaled by the destructor, need to exit immediately
         break;

      // Access logs queue and append new information to output string
      processLogs();
      
      // Release the mutex as soon as possible to prevent delays in other threads
      lk.unlock();

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
      // Open file, write, and close so that user can view update
      theOutputFile.open(theFileName, std::ios::app);
      theOutputFile << theLogToWrite;
      theOutputFile.close();
   }
   theLogToWrite.clear();
}