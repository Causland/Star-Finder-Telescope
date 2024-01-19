#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <string>
#include <string_view>
#include <thread>

/*!
 * Logging macros for easy logging
 */
#define __FILENAME__ (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_ERROR(msg) Logger::log(__FILENAME__, __LINE__, LogCodeEnum::ERROR, msg)
#define LOG_WARN(msg) Logger::log(__FILENAME__, __LINE__, LogCodeEnum::WARNING, msg)
#define LOG_INFO(msg) Logger::log(__FILENAME__, __LINE__, LogCodeEnum::INFO, msg)
#define LOG_DEBUG(msg) Logger::log(__FILENAME__, __LINE__, LogCodeEnum::DEBUG, msg)

/*!
 * Enum class of different log codes to use while logging a message.
 */
enum class LogCodeEnum
{
   ERROR,
   WARNING,
   INFO,
   DEBUG,
};

/*!
 * Generate a string_view for each code in the LogCodeEnum.
 * \param[in] code a LogCodeEnum code for the log.
 * \return a string_view for the provided code.
 */
static constexpr std::string_view logCodeToString(const LogCodeEnum code)
{
   switch (code)
   {
      case (LogCodeEnum::ERROR)   : return "ERROR";
      case (LogCodeEnum::WARNING) : return "WARN";
      case (LogCodeEnum::INFO)    : return "INFO";
      case (LogCodeEnum::DEBUG)   : return "DEBUG";
   } 
   return "?????";
}

/*!
 * The Logger class defines a simple static logger for storing information about different events
 * within the system. The logger is intended to be used for tracking specific information, warnings about
 * system performance, or to indicate if problems have occured. The logger uses a synchronized
 * queue to collect logs from multiple threads and writes them to the target log file. The log file
 * is opened and closed each time to provide live updates to the user.
 */
class Logger
{
public:
   /*!
    * Initializes the logger with a particular stream. The logging thread is started upon initialization.
    * and is only stopped when terminate() is called.
    * \param[in] stream a ostream shared pointer.
    */
   static void initialize(std::shared_ptr<std::ostream> stream);

   /*!
    * Terminates a logger by signaling the condition variable to stop the thread loop. The function
    * blocks on the thread join and writes any remaining logs to the file.
    * \sa processLogs()
    */
   static void terminate();

   /*!
    * Add a new log to the logging queue with the subsystem name, the log code, and a specific message.
    * \param[in] fileName a string_view to the file which called the log.
    * \param[in] lineNum a constant integer to the log line number.
    * \param[in] code a constant LogCodeEnum code.
    * \param[in] msg a string_view of the message to be logged.
    */
   static void log(std::string_view fileName, const uint32_t& lineNum,
                   const LogCodeEnum code, std::string_view msg);

   /*!
    * Generate a string of the log to output to the log file. Log strings are in the format of:
    * DATETIME | [LOG_CODE] MESSAGE (FILE:LINE) where date time is a formatted value based on the time
    * the LogMessage was created.
    * \param[in] fileName a string_view to the file which called the log.
    * \param[in] lineNum a constant integer to the log line number.
    * \param[in] code a constant LogCodeEnum code.
    * \param[in] msg a string_view of the message to be logged.
    * \param[in] timePoint a time point for the log time.
    * \return a string of the msg in log file format.
    */
   [[nodiscard]] static std::string logToString(std::string_view fileName, const uint32_t& lineNum,
                                                const LogCodeEnum code, std::string_view msg,
                                                const std::chrono::time_point<std::chrono::system_clock>& timePoint);

private:
   /*!
    * The logging thread is signaled when a new log is added to the logging queue. The thread removes each log from
    * the queue and concatenates it to an output string. The string is then written to the log file. The
    * mutex is only held while reading from the queue to copy to the output string. This prevents other threads from
    * waiting for the file IO operation.
    * \sa processLogs(), writeToLog()
    */
   static void threadLoop();

   /*!
    * This function removes pending log strings from the log queue and appends them to the output string.
    * The output string is used to write directly to the log file. Once emptied, the condition variable is
    * reset to block the thread until a new log is added.
    * \sa writeToLog()
    */
   static void processLogs();

   /*!
    * This function writes the output string to the log file. The file is opened and closed to allow the user
    * to view the changes as the application is running. The output string is cleared after the write.
    */
   static void writeToLog();

   // NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
   static std::shared_ptr<std::ostream> theOutputStream; //!< The output stream to write to the log file.
   static std::string theLogToWrite; //!< The output string used to write to the log file.
   static bool theLogsAvailableFlag; //!< The flag used to release the condition variable in the logger thread loop.
   static bool theExitingFlag; //!< The flag used to control the thread loop. True when object is begin destroyed.
   static std::condition_variable theCondVar; //!< The condition variable used to block thread until logs are available for processing.
   static std::mutex theMutex; //!< The mutex used to guard the log queue during access by subsystem threads and logging thread.
   static std::thread theThread; //!< The thread for the logger.
   static std::queue<std::string> theLogsToRecord; //!< The queue which holds the pending log strings to write to the log file.
   static bool theInitializedFlag; //!< Indicates that the logger is initialized.
   // NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
};

#endif
