#ifndef OPTICS_MANAGER_HPP
#define OPTICS_MANAGER_HPP

#include "interfaces/IOpticsManager.hpp"

class OpticsManager : public IOpticsManager
{
    // Things for the optics manager to do
    // - Process requests for photos, videos, or timelapse
    //      - Photos
    //          - Focus camera using the Motion Controller
    //          - Take a photo of the target
    //          - Store photo in a known location based on time and target
    //          - Display to output about new photo (Possibly display photo to take another)
    //      - Video
    //          - Focus camera using the Motion Controller
    //          - Take a video of the target
    //          - Store video in a known location based on time and target
    //              - Possibly break up file depending on size
    //          - Display to output about video path
    //      - Timelapse
    //          - Focus camera using the Motion Controller
    //          - Take a photo of the target at specific timelapse frequency
    //          - Store photos in a known location based on timelapse/target/time
    // - Possible to focus camera based on distance to target and focal length of the camera/telescope
};

#endif