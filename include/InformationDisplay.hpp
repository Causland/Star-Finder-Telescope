#ifndef INFORMATION_DISPLAY_HPP
#define INFORMATION_DISPLAY_HPP

#include "Common.hpp"
#include "Subsystem.hpp"
#include "interfaces/GpsModule/IGpsModule.hpp"
#include "interfaces/MotionController/IMotionController.hpp"
#include "interfaces/StarDatabase/IStarDatabase.hpp"
#include <chrono>
#include <fstream>

constexpr std::chrono::milliseconds DEFAULT_DISPLAY_REFRESH_RATE{500};
const std::string DISPLAY_OUTPUT_FILE{"telem_display.txt"};

/*!
 * The InformationDisplay subsystem is responsible for receiving display information
 * from the other subsystems as well as gathering display information from 
 * the external device interfaces. The InformationDiplay threadloop operates
 * by waking up at a set display refresh period to update the output file with
 * new information.
 */
class InformationDisplay : public Subsystem
{
public:
    /*!
     * Creates an InformationDisplay subsystem object with a pointer to
     * each external device to gather information from.
     * \param[in] subsystemName a string of the subsystem name moved into the object.
     * \param[in] starDatabase a pointer to a Star Database interface.
     * \param[in] gpsModule a pointer to a GPS module interface.
     * \param[in] motionController a pointer to a Motion Controller interface.
     */
    InformationDisplay(std::string subsystemName, std::shared_ptr<IStarDatabase> starDatabase, 
                        std::shared_ptr<IGpsModule> gpsModule, std::shared_ptr<IMotionController> motionController) : 
                            Subsystem(subsystemName), myStarDatabase(starDatabase), 
                            myGpsModule(gpsModule), myMotionController(motionController) {};

    /*!
     * Initialize the subsystem and start the display thread.
     */
    void start() override;

    /*!
     * Stop the display thread.
     */
    void stop() override;

    /*!
     * Set interface pointers for use throughout the subsystem.
     * \param[in] subsystems a list of subsystem interface pointers.
     */
    void configureSubsystems(const std::vector<std::shared_ptr<Subsystem>>& subsystems) override {};

    /*!
     * Update the current position and velocity on the display.
     * \param[in] pos a position.
     * \param[in] vel a velocity.
     */
    void updateMotion(const Position& pos, const Velocity& vel);

    /*!
     * Update the search results listed on the display.
     * \param[in] searchResults a string of the results.
     */
    void updateSearchResults(const std::string& searchResults);

    /*!
     * Update the last command listed on the display.
     * \param[in] command a string of the command.
     */
    void updateLastCommand(const std::string& command);

    static const std::string NAME; //!< Name of the subsystem.
private:
    /*!
     * The InformationDisplay threadloop waits for the hearbeat interval or the next refresh
     * time based on the configured refresh period. If the refresh period is reached, the
     * thread calls the updateDisplay() function.
     * \sa updateDisplay()
     */
    void threadLoop() override;

    /*!
     * Update the display file with the information received and gathered from the other
     * subsystems and external interfaces.
     */
    void updateDisplay();

    const std::chrono::system_clock::time_point myStartTime{std::chrono::system_clock::now()}; //!< The start time of the telescope.
    std::ofstream myDisplayFile{DISPLAY_OUTPUT_FILE}; //!< The output display file path.
    std::chrono::milliseconds myRefreshRate{DEFAULT_DISPLAY_REFRESH_RATE}; //!< The display refresh rate.
    std::chrono::system_clock::time_point myLastRefreshTime{std::chrono::system_clock::now()}; //!< The time of the last refresh.
    std::chrono::milliseconds myTimeToRefresh{0}; //!< The time until the next refresh.
    Position myPosition; //!< The current position of the telescope.
    Velocity myVelocity; //!< The current velocity of the telescope.
    std::string mySearchResults; //!< The last search results from the database.
    std::string myLastCommand; //!< The last command from the terminal.
    std::shared_ptr<IStarDatabase> myStarDatabase; //!< The Star Database interface pointer.
    std::shared_ptr<IGpsModule> myGpsModule; //!< The GPS Module interface pointer.
    std::shared_ptr<IMotionController> myMotionController; //!< The Motion Controller interface pointer.
};

#endif