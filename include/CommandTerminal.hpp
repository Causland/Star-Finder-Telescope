#ifndef COMMAND_TERMINAL_HPP
#define COMMAND_TERMINAL_HPP

#include "InformationDisplay.hpp"
#include "OpticsManager.hpp"
#include "PositionManager.hpp"
#include "StarTracker.hpp"
#include "Subsystem.hpp"

#include <array>
#include <atomic>
#include <istream>
#include <memory>
#include <queue>
#include <string>
#include <thread>

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
    * \param[out] exitingSignal a shared pointer to an atomic bool used to signal that the application should close.
    */
   explicit CommandTerminal(std::shared_ptr<std::atomic<bool>> exitingSignal) : 
      Subsystem{"CommandTerminal"}, myExitingSignal{std::move(exitingSignal)} {}

   /*!
    * Destroys a CommandTerminal.
    */
   ~CommandTerminal() override = default;

   CommandTerminal& operator=(const CommandTerminal&) = delete;
   CommandTerminal(const CommandTerminal&) = delete;
   CommandTerminal(CommandTerminal&&) = delete;
   CommandTerminal& operator=(CommandTerminal&&) = delete;

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

protected:
   /*!
    * The CommandTerminal threadloop handles the processing of all incoming commands. It pops pending
    * commands from the command queue, interprets each input command string, validates the formatting of each
    * string, and passes the final command object to the cooresponding subsystem for further processing.
    */
   void threadLoop() override;
 
private:
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
    * \param[in] commandStream a stream with the command to be interpreted.
    */
   void interpretCommand(std::istream& commandStream);

   /*!
    * Perform parameter validation of the input stream for the command. Then perform the required call.
    * \param[in] commandStream a stream with the parameters to be processed.
    */ 
   //@{
   void processPhotoCmd(std::istream& commandStream);
   void processVideoCmd(std::istream& commandStream);
   void processTimelapseCmd(std::istream& commandStream);
   void processMoveCmd(std::istream& commandStream);
   void processFocusCmd(std::istream& commandStream);
   void processFollowCmd(std::istream& commandStream);
   void processGoToCmd(std::istream& commandStream);
   void processSearchCmd(std::istream& commandStream);
   void processCalibrateCmd(std::istream& commandStream);
   //@}

   std::queue<std::string> myCommandQueue{}; //!< The queue which holds all pending commands.
   std::thread myInputWaitingThread{}; //!< The thread used for monitoring of std::cin.
   std::shared_ptr<std::atomic<bool>> myExitingSignal; //!< The flag used to control the lifetime of the application.

   std::weak_ptr<InformationDisplay> myInformationDisplay; //!< Weak pointer to the Information Display.
   std::weak_ptr<OpticsManager> myOpticsManager; //!< Weak pointer to the Optics Manager.
   std::weak_ptr<PositionManager> myPositionManager; //!< Weak pointer to the Position Manager.
   std::weak_ptr<StarTracker> myStarTracker; //!< Weak pointer to the Star Tracker.
};

#endif
