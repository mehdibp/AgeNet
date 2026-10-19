#include "logging/NeighborCsvLogger.h"
#include <filesystem>
#include <iomanip>

namespace veins {


// Open a separate CSV file for this vehicle -------------------------------------------------
void NeighborCsvLogger::open(std::string vehicleName) {
    // node[11] -> node_11, so it is a convenient filename.
    for (char& character : vehicleName) {
        if (character == '[' || character == ']') character = '_';
    }

    const std::filesystem::path path = "results/neighbors_" + vehicleName + ".csv";

    std::error_code ignored;
    std::filesystem::create_directories(path.parent_path(), ignored);

    file.open(path, std::ios::out);
    if (!file.is_open()) {
        throw omnetpp::cRuntimeError("Cannot open neighbor CSV file: %s", path.c_str());
    }

    file << "time,neighbor_vehicle_id,neighbor_id,neighbor_x,neighbor_y,neighbor_speed,distance,last_seen,age\n";
    file << std::setprecision(12);
}

// Write the current neighbor table as a CSV snapshot ----------------------------------------
void NeighborCsvLogger::write(const VehicleState& self, const std::map<int,VehicleState>& neighbors, omnetpp::simtime_t now) {
    if (!file.is_open()) return;

    for (const auto& [id, neighbor] : neighbors) {
        const double distance = self.position.distance(neighbor.position);
        const omnetpp::simtime_t age = now - neighbor.lastSeen;

        file
            << now << ","
            << neighbor.vehicleId << ","
            << id << ","
            << neighbor.position.x << ","
            << neighbor.position.y << ","
            << neighbor.speed << ","
            << distance << ","
            << neighbor.lastSeen << ","
            << age << "\n";
    }

    file.flush();
}

} // namespace veins
