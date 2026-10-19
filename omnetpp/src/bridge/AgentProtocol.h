#pragma once
#include "applications/VehicleState.h"
#include <nlohmann/json.hpp>
#include <string>

namespace veins {

// What Python decided for one vehicle.
struct AgentAction {
    std::string vehicleId;
    double      txPower = 0.0;
};


// The wire format between AgeNetApp and the Python side lives ONLY here.
// (PythonBridge = transport, this file = message schema, AgeNetApp = scheduling.)
// If the JSON fields change on the Python side, this is the single file to edit.
namespace agent_protocol {

    // {"type":"observe", "vehicle_id":..., "sim_time":..., "position":{x,y}, "speed":..., "road_id":..., "neighbors":[...]}
    nlohmann::json buildObservation(const VehicleState& self, const std::map<int, VehicleState>& neighbors, omnetpp::simtime_t now);

    // Validates {"type":"action", "vehicle_id":..., "txPower_mW":...}.
    // Throws std::runtime_error with a readable message on ANY problem.
    AgentAction parseAction(const nlohmann::json& message, const std::string& expectedVehicleId);

} // namespace agent_protocol
} // namespace veins
