#include "AgentProtocol.h"
#include <cmath>
#include <stdexcept>

namespace veins {
namespace agent_protocol {

using json = nlohmann::json;


json buildVehicleEntry(const VehicleState& self, const std::map<int, VehicleState>& neighbors, omnetpp::simtime_t now) {
    json entry = {
        {"position",  json::array({self.position.x, self.position.y})},
        {"speed",     self.speed},
        {"road_id",   self.roadId},
        {"neighbors", json::array()}
    };

    for (const auto& [senderId, neighbor] : neighbors) {
        const double distance = self.position.distance(neighbor.position);
        const omnetpp::simtime_t age = now - neighbor.lastSeen;

        entry["neighbors"].push_back({
            {"sender_id",  senderId},
            {"vehicle_id", neighbor.vehicleId},
            {"position",   json::array({neighbor.position.x, neighbor.position.y})},
            {"speed",      neighbor.speed},
            {"distance",   distance},
            {"age",        age.dbl()},
            {"road_id",    neighbor.roadId}
        });
    }

    return entry;
}

json buildObservation(long step, omnetpp::simtime_t now, const json& sections) {
    json observation = {
        {"type",     "observe"},
        {"step",     step},
        {"sim_time", now.dbl()}
    };

    for (const auto& [name, content] : sections.items()) observation[name] = content;
    return observation;
}

ActionMap parseActions(const json& message, long expectedStep, const std::set<std::string>& knownVehicles) {
    try {
        if (message.value("type", std::string()) != "action") {
            throw std::runtime_error("Python returned an error or unexpected message: " + message.dump());
        }

        const long step = message.at("step").get<long>();
        if (step != expectedStep) {
            throw std::runtime_error("Step mismatch: expected " + std::to_string(expectedStep) + ", received " + std::to_string(step));
        }

        ActionMap actions;
        for (const auto& [vehicleId, entry] : message.at("actions").items()) {
            if (knownVehicles.count(vehicleId) == 0) {
                throw std::runtime_error("Python returned an action for unknown vehicle: " + vehicleId);
            }

            const double txPower = entry.at("txPower_mW").get<double>();
            if (!std::isfinite(txPower) || txPower <= 0.0) {
                throw std::runtime_error("Invalid txPower for " + vehicleId + ": " + std::to_string(txPower));
            }

            actions[vehicleId] = AgentAction{txPower};
        }

        return actions;
    }
    catch (const json::exception& error) {
        throw std::runtime_error(std::string("Invalid action JSON from Python: ") + error.what());
    }
}

} // namespace agent_protocol
} // namespace veins
