#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

enum class LogCodeEnum
{
   ERROR,
   WARNING,
   INFO,
};

struct LogMessage
{
   LogMessage(std::string subsystemName, const LogCodeEnum code, std::string message) : 
      subsystemName(std::move(subsystemName)), code(code), message(std::move(message)), time(std::chrono::system_clock::now()) {}
   
   inline std::string toString()
   {
      // Logs are in format: DATETIME | SUBSYSTEM_NAME [LOG_CODE] MESSAGE
      std::ostringstream oss;
      std::time_t t = std::chrono::system_clock::to_time_t(time);
      oss << std::put_time(std::localtime(&t), "%Y/%m/%d %I:%M:%S %p") 
         << " | "
         << subsystemName
         << " "
         << "[" + logCodeToString(code) + "]"
         << " "
         << message
         << "\n";
      return oss.str();
   }

   inline static std::string logCodeToString(const LogCodeEnum code)
   {
      switch (code)
      {
         case (LogCodeEnum::ERROR)   : return "ERROR";
         case (LogCodeEnum::WARNING) : return "WARNING";
         case (LogCodeEnum::INFO)    : return "INFO";
         default                     : return "UNKNOWN";
      } 
   }
   
   const std::string subsystemName;
   const LogCodeEnum code;
   const std::string message;
   const std::chrono::time_point<std::chrono::system_clock> time;
};

class Logger
{
   public:
      explicit Logger(std::string fileName) : fileName(std::move(fileName)) 
      {
         thread = std::thread(&Logger::threadLoop, this);
      }

      ~Logger()
      {
         exiting = true;
         logsAvailable = true;
         condVar.notify_one();
         thread.join();
         processLogs();
         outputFile.close();
      }

      void log(const std::string& subsystemName, const LogCodeEnum& code, const std::string& message);

   private:
      void threadLoop();
      void processLogs();
      void writeToLog();

      const std::string fileName;
      std::ofstream outputFile;
      std::string logToWrite;
      bool logsAvailable{false};
      bool exiting{false};
      std::condition_variable condVar;
      std::mutex mutex;
      std::thread thread;
      std::queue<std::string> logsToRecord;
};



#endif