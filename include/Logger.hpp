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

/*!
Enum class of different log codes to use while logging a message.
*/
enum class LogCodeEnum
{
   ERROR,
   WARNING,
   INFO,
};

/*!
Structure to hold information related to the logged message. Contains details related to the
time logged, the subsystem, log code, and the log itself.
*/
struct LogMessage
{
   /*!
   Creates a LogMessage object with the specific timestamp of when it is created.
   \param subsystemName [in] a string of the subsystem name moved into structure.
   \param code [in] a constant LogCodeEnum code.
   \param message [in] a string of the message to be logged moved into structure.
   */
   LogMessage(std::string subsystemName, const LogCodeEnum code, std::string message) : 
      subsystemName(std::move(subsystemName)), code(code), message(std::move(message)), time(std::chrono::system_clock::now()) {}
   
   /*!
   Generate a string of the LogMessage to output to the log file. Log strings are in the format of:
   DATETIME | SUBSYSTEM_NAME [LOG_CODE] MESSAGE where date time is a formatted value based on the time
   the LogMessage was created.
   \return a string of the LogMessage in log file format.
   */
   inline std::string toString()
   {
      // Logs are in format: DATETIME | SUBSYSTEM_NAME [LOG_CODE] MESSAGE
      std::ostringstream oss;
      std::time_t t = std::chrono::system_clock::to_time_t(time);
      oss << std::put_time(std::localtime(&t), "%T") 
         << " | "
         << subsystemName
         << " "
         << "[" + logCodeToString(code) + "]"
         << " "
         << message
         << "\n";
      return oss.str();
   }

   /*!
   Generate a string for each code in the LogCodeEnum.
   \param code [in] a LogCodeEnum code for the log.
   \return a string for the provided code.
   */
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
   
   const std::string subsystemName; //!< Name of the subsystem creating the log
   const LogCodeEnum code; //!< Code of the log
   const std::string message; //!< Message of the log
   const std::chrono::time_point<std::chrono::system_clock> time; //!< Timestamp of when the log was created
};

/*!
The Logger class defines a simple logger for storing information about different events
within the system. The logger is intended to be used for tracking specific information, warnings about
system performance, or to indicate if problems have occured. The logger uses a synchronized
queue to collect logs from multiple threads and writes them to the target log file. The log file
is opened and closed each time to provide live updates to the user.
*/
class Logger
{
   public:
      /*!
      Creates a logger with a particular log file. The logging thread is started upon creation.
      \param fileName [in] a string of the relative path to the log file. The string is moved into the class
      */
      explicit Logger(std::string fileName) : fileName(std::move(fileName)) 
      {
         thread = std::thread(&Logger::threadLoop, this);
      }

      /*!
      Destroys a logger by signaling the condition variable to terminate the thread loop. The destructor
      blocks on the thread join and writes the last remaining logs to the file.
      \sa processLogs()
      */
      ~Logger()
      {
         exiting = true;
         logsAvailable = true;
         condVar.notify_one();
         thread.join();
         processLogs();
      }

      Logger(const Logger&) = delete;
      Logger(Logger&&) = delete;
      void operator=(const Logger&) = delete;
      void operator=(Logger&&) = delete;

      /*!
      Add a new log to the logging queue with the subsystem name, the log code, and a specific message.
      \param subsystemName [in] a constant string of the subsystem name.
      \param code [in] a constant LogCodeEnum code.
      \param message [in] a constant string of the message to be logged.
      */
      void log(const std::string& subsystemName, const LogCodeEnum& code, const std::string& message);

   private:
      /*!
      The logging thread is signaled when a new log is added to the logging queue. The thread removes each log from
      the queue and concatenates it to an output string. The string is then written to the log file. The
      mutex is only held while reading from the queue to copy to the output string. This prevents other threads from
      waiting for the file IO operation.
      \sa processLogs(), writeToLog()
      */
      void threadLoop();

      /*!
      This function removes pending log strings from the log queue and appends them to the output string.
      The output string is used to write directly to the log file. Once emptied, the condition variable is
      reset to block the thread until a new log is added.
      \sa writeToLog();
      */
      void processLogs();

      /*!
      This function writes the output string to the log file. The file is opened and closed to allow the user
      to view the changes as the application is running. The output string is cleared after the write.
      */
      void writeToLog();

      const std::string fileName; //!< Relative path to the log file.
      std::ofstream outputFile; //!< The output file stream to write to the log file.
      std::string logToWrite; //!< The output string used to write to the log file.
      bool logsAvailable{false}; //!< The flag used to release the condition variable in the logger thread loop.
      bool exiting{false}; //!< The flag used to control the thread loop. True when object is begin destroyed.
      std::condition_variable condVar; //!< The condition variable used to block thread until logs are available for processing.
      std::mutex mutex; //!< The mutex used to guard the log queue during access by subsystem threads and logging thread.
      std::thread thread; //!< The thread for the logger.
      std::queue<std::string> logsToRecord; //!< The queue which holds the pending log strings to write to the log file.
};

#endif