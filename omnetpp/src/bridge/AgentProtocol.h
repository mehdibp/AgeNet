#pragma once
#include "applications/VehicleState.h"
#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <string>

namespace veins {

// What Python decided for one vehicle.
struct AgentAction {
    double radius = 0.0;    // m
};
using ActionMap = std::map<std::string, AgentAction>;   // vehicle_id -> action


// The wire format between the Coordinator and Python lives ONLY here.
namespace agent_protocol {

    // v2: actions carry "radius_m" (v1 carried "txPower_mW"); hello carries "radius_range_m".
    constexpr int PROTOCOL_VERSION = 2;

    // Static facts, sent exactly once right after connecting. radius range = [radius of minTxPower, radius of maxTxPower]
    nlohmann::json buildHello(double playgroundX, double playgroundY, double cyclePeriod, double minRadius, double maxRadius);
    void checkHelloAck(const nlohmann::json& message);   // throws std::runtime_error

    // {"position":[x,y], "speed":..., "road_id":..., "neighbors":[...]}
    nlohmann::json buildVehicleEntry(const VehicleState& self, const std::map<int, VehicleState>& neighbors, 
                                     omnetpp::simtime_t now, double txPowerMw, double radius);

    // Whole message. `sections` is an object like {"vehicles": {...}, "traffic_lights": {...}};
    nlohmann::json buildObservation(long step, omnetpp::simtime_t now, const nlohmann::json& sections);

    // Validates {"type":"action","step":..., "actions":{vid:{"txPower_mW":..}}}.
    ActionMap parseActions(const nlohmann::json& message, long expectedStep, const std::set<std::string>& knownVehicles);

} // namespace agent_protocol
} // namespace veins

