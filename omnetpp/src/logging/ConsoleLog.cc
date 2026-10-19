#include "logging/ConsoleLog.h"

#if AGENET_QTENV_LOG

#include <omnetpp.h>

using namespace omnetpp;

namespace veins {
namespace console_log {


namespace {
    // One vehicle's state on a single line.
    void printLog(const std::string& prefix, const std::string& details, const VehicleState& state) {
        EV_INFO
            << prefix << "--"
            << " vehicle: " << state.vehicleId
            << " | time: "  << simTime()
            << " | position: (" << state.position.x << ", " << state.position.y << ")"
            << " | speed: " << state.speed << " m/s"
            << " | road: "  << state.roadId;

        if (!details.empty()) EV_INFO << " | " << details;
        EV_INFO << endl;
    }

} // namespace

void status(const VehicleState& self, const std::string& prefix) {
    printLog(prefix, "", self);
}

void transmit(const VehicleState& self, int serial) {
    printLog("TX", "serial: " + std::to_string(serial), self);
}

void received(const std::string& host, const std::string& fromVehicle, std::size_t neighborCount) {
    EV_INFO
        << "[RX] Vehicle: "    << host
        << " | received from " << fromVehicle
        << " | neighbors: "    << neighborCount
        << endl;
}

void neighborLost(const std::string& host, const VehicleState& lost) {
    EV_INFO
        << "[NEIGHBOR LOST] " << host
        << " lost " << lost.vehicleId
        << " at "   << simTime()
        << endl;
}

void neighborTable(const VehicleState& self, const std::map<int, VehicleState>& neighbors) {
    EV_INFO
        << "[NEIGHBORS] "
        << "vehicle: "  << self.vehicleId
        << " | time: "  << simTime()
        << " | count: " << neighbors.size()
        << endl;

    if (neighbors.empty()) {
        EV_INFO << "    (no neighbors)" << endl;
        return;
    }

    for (const auto& [id, neighbor] : neighbors) {
        const double distance = self.position.distance(neighbor.position);
        const omnetpp::simtime_t age = simTime() - neighbor.lastSeen;

        printLog("    ",
                 "id: " + std::to_string(id) +
                 " | distance: " + std::to_string(distance) + " m" +
                 " | age: " + std::to_string(age.dbl()) + " s",
                 neighbor);
    }
}

void pythonAction(const VehicleState& self, const double txPowerMw, std::size_t neighborCount) {
    EV_INFO
        << "[PYTHON ACTION] vehicle=" << self.vehicleId
        << " | time=" << simTime()
        << " | txPower=" << txPowerMw << " mW"
        << " | neighbors=" << neighborCount
        << endl;
}


} // namespace console_log
} // namespace veins

#endif // AGENET_QTENV_LOG
