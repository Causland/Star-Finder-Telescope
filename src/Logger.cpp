#include "Logger.hpp"
#include <iostream>

void Logger::log(const std::string& subsystemName, const LogCodeEnum& code, const std::string& message)
{
   LogMessage messageToLog(subsystemName, code, message);
   std::string logString = messageToLog.toString();
   {
      std::scoped_lock<std::mutex> lk(mutex);
      logsToRecord.push(logString);
      logsAvailable = true;
   }
   condVar.notify_one();
}

void Logger::threadLoop()
{
   while (!exiting)
   {
      // Wait until log is written into the incomingLogs queue
      std::unique_lock<std::mutex> lk(mutex);
      condVar.wait(lk, [this]{ return logsAvailable; });
      if (exiting)
         break;

      // Access incoming logs queue and write to file
      processLogs();
      
      // Release the mutex as soon as possible to allow for other logs
      lk.unlock();

      // Write all the logs accumulated to the output file
      writeToLog();
   }
}

void Logger::processLogs()
{
   while (!logsToRecord.empty())
   {
      logToWrite += logsToRecord.front();
      logsToRecord.pop();
   }
   logsAvailable = false;
}

void Logger::writeToLog()
{
   if (!logToWrite.empty())
   {
      outputFile.open(fileName, std::ios::app);
      outputFile << logToWrite;
      outputFile.close();
   }
   logToWrite.clear();
}
