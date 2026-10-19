#include "AgentProtocol.h"
#include <cmath>
#include <stdexcept>

namespace veins {
namespace agent_protocol {

using json = nlohmann::json;


json buildObservation(const VehicleState& self, const std::map<int, VehicleState>& neighbors, omnetpp::simtime_t now) {
    json observation = {
        {"type",       "observe"},
        {"vehicle_id", self.vehicleId},
        {"sim_time",   now.dbl()},
        {"position",   {{"x", self.position.x}, {"y", self.position.y}}},
        {"speed",      self.speed},
        {"road_id",    self.roadId},
        {"neighbors",  json::array()}
    };

    for (const auto& [senderId, neighbor] : neighbors) {
        const double distance = self.position.distance(neighbor.position);
        const omnetpp::simtime_t age = now - neighbor.lastSeen;

        observation["neighbors"].push_back({
            {"sender_id",  senderId},
            {"vehicle_id", neighbor.vehicleId},
            {"x",          neighbor.position.x},
            {"y",          neighbor.position.y},
            {"speed",      neighbor.speed},
            {"distance",   distance},
            {"age",        age.dbl()},
            {"road_id",    neighbor.roadId}
        });
    }

    return observation;
}


AgentAction parseAction(const json& message, const std::string& expectedVehicleId) {
    try {
        if (message.value("type", std::string()) != "action") {
            throw std::runtime_error("Python returned an error or unexpected message: " + message.dump());
        }

        AgentAction action;
        action.vehicleId = message.at("vehicle_id").get<std::string>();
        if (action.vehicleId != expectedVehicleId) {
            throw std::runtime_error("Python action vehicle mismatch: expected " + expectedVehicleId + ", received " + action.vehicleId);
        }

        action.txPower = message.at("txPower_mW").get<double>();
        if (!std::isfinite(action.txPower) || action.txPower <= 0.0) {
            throw std::runtime_error("Invalid txPower returned by Python: " + std::to_string(action.txPower));
        }

        return action;
    }
    catch (const json::exception& error) {
        throw std::runtime_error(std::string("Invalid action JSON from Python: ") + error.what());
    }
    
}

} // namespace agent_protocol
} // namespace veins
