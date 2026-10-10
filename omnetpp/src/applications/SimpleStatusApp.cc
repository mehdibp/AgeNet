#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "messages/VehicleStatusMessage_m.h"
#include "SimpleStatusApp.h"

#include <iomanip>
#include <cmath>


using namespace veins;
using namespace omnetpp;

Define_Module(SimpleStatusApp);


// -------------------------------------------------------------------------------------------
// Initialize the application ----------------------------------------------------------------
void SimpleStatusApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        openNeighborCSV();
        
        scheduleAt(simTime() + SimTime(1, SIMTIME_S), new cMessage("statusTimer"));
        scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), new cMessage("sendTimer"));
        scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), new cMessage("neighborTimer"));
    }
}

// Finish the application --------------------------------------------------------------------
void SimpleStatusApp::finish() {
    if (neighborCsv.is_open()) neighborCsv.close();
    DemoBaseApplLayer::finish();
}


// -------------------------------------------------------------------------------------------
// Read the current vehicle state ------------------------------------------------------------
SimpleStatusApp::VehicleState
SimpleStatusApp::_getVehicleState() {
    TraCIMobility* mobility = TraCIMobilityAccess().get(findHost());
    VehicleState state;

    state.vehicleId = findHost()->getFullName();
    state.position  = mobility->getPositionAt(simTime());
    state.speed     = mobility->getSpeed();
    state.roadId    = mobility->getRoadId();

    return state;
}

// Calculate Euclidean distance between two positions ----------------------------------------
double SimpleStatusApp::_getDistance(double x1, double y1, double x2, double y2) const {
    const double dx = x2 - x1;
    const double dy = y2 - y1;

    return std::sqrt(dx * dx + dy * dy);
}

// Print a vehicle's current state -----------------------------------------------------------
void SimpleStatusApp::_printLog(const std::string& prefix, const std::string& details, const VehicleState& state) {
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


// -------------------------------------------------------------------------------------------
// Handle self-messages ----------------------------------------------------------------------
void SimpleStatusApp::handleSelfMsg(cMessage* msg) {

    if (msg->isName("statusTimer")) {
        // VehicleState state = getVehicleState();
        // printLog("VEHICLE", "", state);
        // scheduleAt(simTime() + SimTime(1, SIMTIME_S), msg);
    }
    else if (msg->isName("sendTimer")) {
        sendVehicleStatus();
        scheduleAt(simTime() + SimTime(1, SIMTIME_S) + uniform(0.01, 0.5), msg);
    }
    else if (msg->isName("neighborTimer")) {
        updateNeighbors();
        printNeighborTable();
        saveNeighborsToCSV();
        scheduleAt(simTime() + SimTime(1, SIMTIME_S), msg);
    }
    else { DemoBaseApplLayer::handleSelfMsg(msg); }

}

// Handle received messages ------------------------------------------------------------------
void SimpleStatusApp::onWSM(BaseFrame1609_4* wsm) {
    VehicleStatusMessage* message = check_and_cast<VehicleStatusMessage*>(wsm);

    int senderId = message->getSenderId();

    // Ignore our own messages.
    if (senderId == myId) return;

    VehicleState neighbor;

    neighbor.vehicleId = message->getVehicleId();
    neighbor.position  = Coord(message->getPosX(), message->getPosY());
    neighbor.speed     = message->getSpeed();
    neighbor.roadId    = message->getRoadId();
    neighbor.lastSeen  = simTime();

    neighbors[senderId] = neighbor;

    EV_INFO
        << "[RX] Vehicle: "    << findHost()->getFullName()
        << " | received from " << neighbor.vehicleId
        << " | neighbors: "    << neighbors.size()
        << endl;
}

// Handle Updating Position ------------------------------------------------------------------
void SimpleStatusApp::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // VehicleState state = getVehicleState();
    // printLog("MOVEMENT", "", state);
}


// -------------------------------------------------------------------------------------------
// Send our vehicle status -------------------------------------------------------------------
void SimpleStatusApp::sendVehicleStatus() {
    VehicleState state = getVehicleState();

    VehicleStatusMessage* message = new VehicleStatusMessage();
    populateWSM(message);

    message->setSenderId(myId);
    message->setVehicleId(state.vehicleId.c_str());
    message->setPosX(state.position.x);
    message->setPosY(state.position.y);
    message->setSpeed(state.speed);
    message->setRoadId(state.roadId.c_str());
    message->setCreationTime(simTime());
    message->setSerial(messageCounter++);

    message->setRecipientAddress(LAddress::L2BROADCAST());      // Broadcast

    printLog("TX", "serial: "+std::to_string(message->getSerial()), state);
    sendDown(message);
}

// Remove neighbors that have timed out ------------------------------------------------------
void SimpleStatusApp::updateNeighbors() {
    for (auto it=neighbors.begin(); it!=neighbors.end();) {
        if (simTime() - it->second.lastSeen > neighborTimeout) {
            EV_INFO
                << "[NEIGHBOR LOST] " << findHost()->getFullName()
                << " lost " << it->second.vehicleId
                << " at "   << simTime()
                << endl;

            it = neighbors.erase(it);
        }
        else {
            ++it;
        }
    }
}

// Print current neighbor table --------------------------------------------------------------
void SimpleStatusApp::printNeighborTable() {
    const VehicleState state = getVehicleState();

    EV_INFO
        << "[NEIGHBORS] "
        << "vehicle: "  << state.vehicleId
        << " | time: "  << simTime()
        << " | count: " << neighbors.size()
        << endl;

    if (neighbors.empty()) {
        EV_INFO << "    (no neighbors)" << endl;
        return;
    }

    // Print every neighbor
    for (const auto& [id, neighbor] : neighbors) {
        const double distance = getDistance(state.position.x, state.position.y, neighbor.position.x, neighbor.position.y);

        simtime_t age = simTime() - neighbor.lastSeen;

        printLog("    ", 
                 "id: " + std::to_string(id) + 
                 " | distance: " + std::to_string(distance) + " m" + 
                 " | age: " + std::to_string(age.dbl()) + " s", 
                 neighbor);
    }

}

// Open a separate CSV file for this vehicle -------------------------------------------------
void SimpleStatusApp::openNeighborCSV() {
    std::string vehicleName = findHost()->getFullName();

    // Convert node[11] to node_11 for a convenient filename.
    for (char& c : vehicleName) {
        if (c == '[' || c == ']')
            c = '_';
    }

    std::string filename = "results/neighbors_" + vehicleName + ".csv";

    neighborCsv.open(filename, std::ios::out);

    if (!neighborCsv.is_open()) {
        throw cRuntimeError("Cannot open neighbor CSV file: %s", filename.c_str());
    }

    neighborCsv << "time,neighbor_vehicle_id,neighbor_id,neighbor_x,neighbor_y,neighbor_speed,distance,last_seen,age\n";
    neighborCsv << std::setprecision(12);
}

// Write the current neighbor table as a CSV snapshot ----------------------------------------
void SimpleStatusApp::saveNeighborsToCSV() {
    if (!neighborCsv.is_open()) return;

    const VehicleState state = getVehicleState();

    for (const auto& [id, neighbor] : neighbors) {
        const double distance = getDistance(state.position.x, state.position.y, neighbor.position.x, neighbor.position.y);

        omnetpp::simtime_t age = simTime() - neighbor.lastSeen;

        neighborCsv
            << simTime() << ","
            << neighbor.vehicleId << ","
            << id << ","
            << neighbor.position.x << ","
            << neighbor.position.y << ","
            << neighbor.speed << ","
            << distance << ","
            << neighbor.lastSeen << ","
            << age << "\n";
    }

    neighborCsv.flush();
}


