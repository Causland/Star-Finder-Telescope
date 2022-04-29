#include "Logger.hpp"
#include <iostream>

void Logger::log(const std::string& subsystemName, const LogCodeEnum& code, const std::string& message)
{
   // Create the log message and get the string representation
   LogMessage messageToLog(subsystemName, code, message);
   std::string logString = messageToLog.toString();
   {
      std::scoped_lock<std::mutex> lk(myMutex);
      myLogsToRecord.push(logString);
      myLogsAvailableFlag = true;
   }
   // Signal to the condition variable to wake up the logging thread
   myCondVar.notify_one();
}

void Logger::threadLoop()
{
   while (!myExitingFlag)
   {
      // Wait until log is written into the queue
      std::unique_lock<std::mutex> lk(myMutex);
      myCondVar.wait(lk, [this]{ return myLogsAvailableFlag; });
      if (myExitingFlag) // If signaled by the destructor, need to exit immediately
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
   while (!myLogsToRecord.empty())
   {
      // Append each log to the output string for writing to log file
      myLogToWrite += myLogsToRecord.front();
      myLogsToRecord.pop();
   }
   myLogsAvailableFlag = false;
}

void Logger::writeToLog()
{
   if (!myLogToWrite.empty())
   {
      // Open file, write, and close so that user can view update
      myOutputFile.open(myFileName, std::ios::app);
      myOutputFile << myLogToWrite;
      myOutputFile.close();
   }
   myLogToWrite.clear();
}
