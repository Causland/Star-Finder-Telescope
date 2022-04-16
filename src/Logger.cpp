#include "Logger.hpp"

void Logger::log(const std::string& subsystemName, const LogCodeEnum& code, const std::string& message)
{
   // Create the log message and get the string representation
   LogMessage messageToLog(subsystemName, code, message);
   auto logString = messageToLog.toString();

   // Get the next write index in the buffer container and atomically increment the value
   auto index = myLogBufferIndex.fetch_add(1) % NUM_LOG_BUFFERS;
   
   // Write string to buffer
   // NOTE - This does not write or notify if all buffers are full. 
   // The number of buffers is dictated the NUM_LOG_BUFFERS constant and should be
   // resized depending on frequency of logging and number of independent threads
   if (myLogsToRecord[index].empty())
   {
      myLogsToRecord[index] = logString;
      // Signal to the condition variable to wake up the logging thread
      ++myNumLogsWaiting;
      myCondVar.notify_one();
   }
}

void Logger::threadLoop()
{
   while (!myExitingFlag)
   {
      // Wait until log is written into the queue
      std::unique_lock<std::mutex> lk(myMutex);
      myCondVar.wait(lk, [this]{ return myNumLogsWaiting > 0; });
      if (myExitingFlag) // If signaled by the destructor, need to exit immediately
         break;

      // Access logs queue and append new information to output string
      processLogs();

      // Write the logs to the output file
      writeToLog();
   }
}

void Logger::processLogs()
{
   // Decrement atomic until no logs are pending in buffers
   while (myNumLogsWaiting.load())
   {
      myLogToWrite += myLogsToRecord[myLogBufferReadIndex];
      myLogsToRecord[myLogBufferReadIndex].clear();
      myLogBufferReadIndex = (myLogBufferReadIndex + 1) % NUM_LOG_BUFFERS;
      --myNumLogsWaiting;
   }
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
