#pragma once
#include "applications/VehicleState.h"
#include <omnetpp.h>
#include <fstream>
#include <string>


namespace veins {

// Writes one CSV per vehicle: a snapshot of its neighbor table every time write() is called.
// The file is closed automatically when the logger is destroyed.
class NeighborCsvLogger {
    public:
        // Creates results/neighbors_<vehicleName>.csv ("node[11]" -> "node_11") and writes the header.
        // Throws omnetpp::cRuntimeError if the file cannot be opened.
        void open(std::string vehicleName);
        void write(const VehicleState& self, const std::map<int, VehicleState>& neighbors, omnetpp::simtime_t now);
        bool isOpen() const { return file.is_open(); }
        void Close() { file.close();}

    private:
        std::ofstream file;
};

} // namespace veins
