#ifndef COMMAND_TERMINAL_HPP
#define COMMAND_TERMINAL_HPP

#include "CommandTypes.hpp"
#include "Logger.hpp"
#include "Subsystem.hpp"
#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <queue>
#include <string>
#include <type_traits>

class InformationDisplay;
class OpticsManager;
class PositionManager;
class StarTracker;

/*!
 * The CommandTerminal subsystem is responsible for processing inputs from the user. There are
 * two threads which provide functionality, the user input monitoring thread and the
 * command processing thread.
 * The user input monitoring thread waits for input in the std::cin stream, splits semicolon delimited
 * inputs apart, and adds each command to the command queue for processing.
 * The command processing thread interprets and validates each command pushed onto the command queue.
 * The commands are passed to the proper subsystem for a coordinating action to occur.
 */
class CommandTerminal : public Subsystem
{
public:
   /*!
    * Creates a CommandTerminal subsystem object with a logger and pointer to the main forever loop controller.
    * \param[in] subsystemName a string of the subsystem name moved into the class.
    * \param[out] exitingSignal a shared pointer to an atomic bool used to signal that the application should close.
    */
   CommandTerminal(const std::string& subsystemName, std::shared_ptr<std::atomic<bool>> exitingSignal) : 
      Subsystem(subsystemName), myExitingSignal(exitingSignal) {}

   /*!
    * Initialize the subsystem and start the threads.
    */
   void start() override;

   /*!
    * Cleanup the subsystem, signal to end the forever loops, and wait for the threads to join.
    */
   void stop() override;

   /*!
    * Set interface pointers for use throughout the subsystem.
    * \param[in] subsystems a list of subsystem interface pointers.
    */
   void configureSubsystems(const std::array<std::shared_ptr<Subsystem>, 
                                             static_cast<size_t>(SubsystemEnum::NUM_SUBSYSTEMS)>& subsystems) override;

   static const std::string NAME; //!< The name of the subsystem.
private:
   /*!
    * The CommandTerminal threadloop handles the processing of all incoming commands. It pops pending
    * commands from the command queue, interprets each input command string, validates the formatting of each
    * string, and passes the final command object to the cooresponding subsystem for further processing.
    */
   void threadLoop() override;
    
   /*!
    * The user input threadloop waits for input from the std::cin stream. The semicolon delimited commands
    * are split and then placed onto the command queue.
    */
   void cinWaitThreadLoop();

   /*!
    * Every command input string should be formatted with a base command keyword followed by a space delimited
    * list of parameters. This function parses out the base command to determine processing of the required
    * command arguments. The command arguments are validated and passed to the dedicated subsystem for further
    * processing.
    * \param[in] command a command string to be interpreted.
    * \return true if the command is successfully interpreted.
    */
   bool interpretCommand(const std::string& command);

   /*!
    * The base case for the recursive variadic function validateParameters(). This function
    * checks the input string to see if it is empty. At the base case, the input string should
    * be completely processed leaving no more characters in the string.
    * \param[in] input a list of remaining parameters for a command.
    * \return true if the input string is empty.
    */
   bool validateParameters(const std::string& input)
   {
      if (!input.empty())
      {
         LOG_ERROR("Too many parameters provided for the command");
         return false;
      }
      return true;
   }

   /*!
    * Every command input string should be formatted with a base command followed by a space delimited list
    * of parameters. This variadic function takes in the space delimited parameter list and a variable number
    * of out parameters. Each parameter in the string is matched to the next function parameter provided.
    * This function recursively unpacks the parameter pack, converts the next parameter to the desired type,
    * and checks to see if enough parameters for the command were provided.
    * 
    * There are three supported types for parameters: std::string, double, and uint64_t.
    * Example Command               -> "video test.jpg 30"
    * Photo command parameter types -> std::string, uint64_t
    * test.jpg is matched with the type std::string.
    * 30 is matched with the type uint64_t and is converted into the type.
    * 
    * \tparam T the type of the current out parameter.
    * \tparam Args... the type of the parameter pack of out parameters.
    * \param[in] input a space delimited string of parameters to process as values.
    * \param[in] currParam the current out parameter to match with the input string.
    * \param[out] outParams a parameter pack of out parameters.
    * \return true if there are no remaining parameters in the input string, no remaining parameters in the out parameters, and all values could be converted.
    */
   template<typename T, typename... Args>
   bool validateParameters(const std::string& input, T& currParam, Args&... outParams)
   {
      if (input.empty())
      {
         LOG_ERROR("Not enough parameters provided for command");
         return false;
      }
      auto pos = input.find(' ');
      std::string val = input.substr(0, pos);
      std::string remainder;
      if (pos == std::string::npos)
      {
         remainder = "";
      }
      else
      {
         remainder = input.substr(pos + 1);
      }

      // Store result of converted parameter in the current parameter
      if (std::is_same_v<decltype(currParam), std::string&>)
      {
         currParam = reinterpret_cast<T&>(val);
      }
      else if (std::is_same_v<decltype(currParam), uint64_t&>)
      {
         char* end = nullptr;
         constexpr auto BASE_TEN = 10;
         uint64_t temp = strtoul(val.c_str(), &end, BASE_TEN);
         if (end == val.c_str() || *end != '\0')
         {
            LOG_ERROR("Parameter " + val + " cannot be converted to long integer type");
            return false;
         }
         currParam = temp;
      }
      else if (std::is_same_v<decltype(currParam), double&>)
      {
         char* end = nullptr;
         double temp = strtod(val.c_str(), &end);
         if (end == val.c_str() || *end != '\0')
         {
            LOG_ERROR("Parameter " + val + " cannot be converted to double type");
            return false;
         }
         currParam = temp;
      }
      else
      {
         LOG_ERROR("Unsupported type " + std::string(typeid(currParam).name()) + " passed into command");
         return false;
      }

      // There are more parameters to unpack
      return validateParameters(remainder, outParams...);
   }

   std::queue<std::string> myCommandQueue{}; //!< The queue which holds all pending commands.
   std::thread myInputWaitingThread{}; //!< The thread used for monitoring of std::cin.
   std::shared_ptr<std::atomic<bool>> myExitingSignal; //!< The flag used to control the lifetime of the application.

   std::weak_ptr<InformationDisplay> myInformationDisplay; //!< Weak pointer to the Information Display.
   std::weak_ptr<OpticsManager> myOpticsManager; //!< Weak pointer to the Optics Manager.
   std::weak_ptr<PositionManager> myPositionManager; //!< Weak pointer to the Position Manager.
   std::weak_ptr<StarTracker> myStarTracker; //!< Weak pointer to the Star Tracker.
};

#endif