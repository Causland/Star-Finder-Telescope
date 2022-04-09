#include "Logger.hpp"
#include <iostream>

void Logger::log(const std::string& subsystemName, const LogCodeEnum& code, const std::string& message)
{
   // Create the log message and get the string representation
   LogMessage messageToLog(subsystemName, code, message);
   std::string logString = messageToLog.toString();
   {
      std::scoped_lock<std::mutex> lk(mutex);
      logsToRecord.push(logString);
      logsAvailable = true;
   }
   // Signal to the condition variable to wake up the logging thread
   condVar.notify_one();
}

void Logger::threadLoop()
{
   while (!exiting)
   {
      // Wait until log is written into the queue
      std::unique_lock<std::mutex> lk(mutex);
      condVar.wait(lk, [this]{ return logsAvailable; });
      if (exiting) // If signaled by the destructor, need to exit immediately
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
   while (!logsToRecord.empty())
   {
      // Append each log to the output string for writing to log file
      logToWrite += logsToRecord.front();
      logsToRecord.pop();
   }
   logsAvailable = false;
}

void Logger::writeToLog()
{
   if (!logToWrite.empty())
   {
      // Open file, write, and close so that user can view update
      outputFile.open(fileName, std::ios::app);
      outputFile << logToWrite;
      outputFile.close();
   }
   logToWrite.clear();
}
