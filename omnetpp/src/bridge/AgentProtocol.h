#pragma once
#include "applications/VehicleState.h"
#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <string>

namespace veins {

// What Python decided for one vehicle.
struct AgentAction {
    double txPower = 0.0;   // mW
};
using ActionMap = std::map<std::string, AgentAction>;   // vehicle_id -> action


// The wire format between the Coordinator and Python lives ONLY here.
// (PythonBridge = transport, this file = schema, AgeNetCoordinator = scheduling.)
namespace agent_protocol {

    // {"position":[x,y], "speed":..., "road_id":..., "neighbors":[...]}
    nlohmann::json buildVehicleEntry(const VehicleState& self, const std::map<int, VehicleState>& neighbors, omnetpp::simtime_t now);

    // Whole message. `sections` is an object like {"vehicles": {...}, "traffic_lights": {...}};
    nlohmann::json buildObservation(long step, omnetpp::simtime_t now, const nlohmann::json& sections);

    // Validates {"type":"action","step":..., "actions":{vid:{"txPower_mW":..}}}.
    ActionMap parseActions(const nlohmann::json& message, long expectedStep, const std::set<std::string>& knownVehicles);

} // namespace agent_protocol
} // namespace veins

