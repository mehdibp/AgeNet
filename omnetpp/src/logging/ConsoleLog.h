#pragma once
// ===========================================================================================
// Human-readable EV_INFO output (what you see in Qtenv / Cmdenv). NOT needed for the simulation
// to work. To remove all of it from the build, compile with  -DAGENET_QTENV_LOG=0  and every
// function below becomes an empty inline no-op (no call site has to change).
// ===========================================================================================
#ifndef AGENET_QTENV_LOG
#define AGENET_QTENV_LOG 1
#endif

#include <cstddef>
#include <string>
#include "applications/VehicleState.h"

namespace veins {
namespace console_log {

#if AGENET_QTENV_LOG

    void status       (const VehicleState& self, const std::string& prefix);
    void transmit     (const VehicleState& self, int serial);
    void received     (const std::string& host, const std::string& fromVehicle, std::size_t neighborCount);
    void neighborLost (const std::string& host, const VehicleState& lost);
    void neighborTable(const VehicleState& self, const std::map<int, VehicleState>& neighbors);
    void pythonAction (const VehicleState& self, const double txPowerMw, std::size_t neighborCount);

#else

    inline void status       (const VehicleState&, const std::string&) {}
    inline void transmit     (const VehicleState&, int) {}
    inline void received     (const std::string&, const std::string&, std::size_t) {}
    inline void neighborLost (const std::string&, const VehicleState&) {}
    inline void neighborTable(const VehicleState&, const std::map<int, VehicleState>&) {}
    inline void pythonAction (const VehicleState&, const double, std::size_t) {}

#endif

} // namespace console_log
} // namespace veins
