#pragma once
#include "veins/base/utils/Coord.h"
#include <string>

namespace veins {

// Snapshot of one vehicle (either "me" or something I heard over the radio).
struct VehicleState {

    std::string vehicleId;
    Coord position;
    double speed;
    std::string roadId;
    omnetpp::simtime_t lastSeen;

};

} // namespace veins
