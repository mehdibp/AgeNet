#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "messages/VehicleStatusMessage_m.h"

#include "SimpleStatusApp.h"
#include "PythonBridge.h"

#include <iomanip>
#include <cmath>
#include <stdexcept>


using namespace veins;
using namespace omnetpp;
using json = nlohmann::json;

Define_Module(SimpleStatusApp);


// -------------------------------------------------------------------------------------------
SimpleStatusApp::~SimpleStatusApp() {
    if (pythonTimer != nullptr) {
        cancelAndDelete(pythonTimer);
        pythonTimer = nullptr;
    }
    pythonBridge.reset();
}

// Initialize the application ----------------------------------------------------------------
void SimpleStatusApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        openNeighborCSV();
        
        scheduleAt(simTime() + SimTime(1, SIMTIME_S), new cMessage("statusTimer"));
        scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), new cMessage("sendTimer"));
        scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), new cMessage("neighborTimer"));


        if (par("pythonBridgeEnabled").boolValue()) {
            const std::string host = par("pythonHost").stdstringValue();
            const int port = par("pythonPort").intValue();

            pythonBridge = std::make_unique<PythonBridge>();
            try { pythonBridge->connectToServer(host, port); }
            catch (const std::exception& error) {
                pythonBridge.reset();
                throw cRuntimeError("Python bridge connection failed for %s: %s", findHost()->getFullName(), error.what());
            }

            EV_INFO << "[PYTHON] Connected for vehicle " << findHost()->getFullName() <<" to "<< host <<":"<< port << endl;

            pythonTimer = new cMessage("pythonTimer");
            scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), pythonTimer);
        }
    }
}

// Finish the application --------------------------------------------------------------------
void SimpleStatusApp::finish() {
    if (pythonTimer != nullptr) {
        cancelAndDelete(pythonTimer);
        pythonTimer = nullptr;
    }
    pythonBridge.reset();

    if (neighborCsv.is_open()) neighborCsv.close();
    DemoBaseApplLayer::finish();
}


// -------------------------------------------------------------------------------------------
// Read the current vehicle state ------------------------------------------------------------
SimpleStatusApp::VehicleState SimpleStatusApp::getVehicleState() {
    TraCIMobility* mobility = TraCIMobilityAccess().get(findHost());
    VehicleState state;

    state.vehicleId = findHost()->getFullName();
    state.position  = mobility->getPositionAt(simTime());
    state.speed     = mobility->getSpeed();
    state.roadId    = mobility->getRoadId();

    return state;
}

// Print a vehicle's current state -----------------------------------------------------------
void SimpleStatusApp::printLog(const std::string& prefix, const std::string& details, const VehicleState& state) {
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

    if (msg->isName("pythonTimer")) {
        exchangeWithPython();
        scheduleAt(simTime() + SimTime(1, SIMTIME_S), msg);
    }
    else if (msg->isName("statusTimer")) {
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
        double distance = state.position.distance(neighbor.position);
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
    for (char& character : vehicleName) {
        if (character == '[' || character == ']') character = '_';
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
        double distance = state.position.distance(neighbor.position);
        simtime_t age = simTime() - neighbor.lastSeen;

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


// Exchange the current observation and receive a test action from Python --------------------
void SimpleStatusApp::exchangeWithPython() {
    if (!pythonBridge || !pythonBridge->isConnected()) {
        throw cRuntimeError("Python bridge is not connected for %s", findHost()->getFullName());
    }

    const VehicleState state = getVehicleState();

    json observation = {
        {"type", "observe"},
        {"vehicle_id", state.vehicleId},
        {"sim_time", simTime().dbl()},
        {"position", {{"x", state.position.x}, {"y", state.position.y}}},
        {"speed", state.speed},
        {"road_id", state.roadId},
        {"neighbors", json::array()}
    };

    for (const auto& [senderId, neighbor] : neighbors) {
        const double distance = state.position.distance(neighbor.position);
        const double age = (simTime() - neighbor.lastSeen).dbl();

        observation["neighbors"].push_back({
            {"sender_id", senderId},
            {"vehicle_id", neighbor.vehicleId},
            {"x", neighbor.position.x},
            {"y", neighbor.position.y},
            {"speed", neighbor.speed},
            {"distance", distance},
            {"age", age},
            {"road_id", neighbor.roadId}
        });
    }

    json action;
    try {
        action = pythonBridge->exchange(observation);
    }
    catch (const std::exception& error) {
        throw cRuntimeError("Python bridge exchange failed for %s: %s", state.vehicleId.c_str(), error.what());
    }

    try {
        if (action.value("type", std::string()) != "action") {
            throw cRuntimeError("Python returned an error or unexpected message for %s: %s",
                                state.vehicleId.c_str(), action.dump().c_str());
        }

        const std::string actionVehicle = action.at("vehicle_id").get<std::string>();
        if (actionVehicle != state.vehicleId) {
            throw cRuntimeError("Python action vehicle mismatch: expected %s, received %s",
                                state.vehicleId.c_str(), actionVehicle.c_str());
        }

        const double txPower = action.at("txPower_mW").get<double>();
        if (!std::isfinite(txPower) || txPower <= 0.0) {
            throw cRuntimeError("Invalid txPower returned by Python: %g", txPower);
        }

        EV_INFO
            << "[PYTHON ACTION] vehicle=" << state.vehicleId
            << " | time=" << simTime()
            << " | txPower=" << txPower << " mW"
            << " | neighbors=" << neighbors.size()
            << endl;

        // Integration stage only: action is validated and logged, but radio
        // transmit power is intentionally not modified yet.
    }
    catch (const nlohmann::json::exception& error) {
        throw cRuntimeError("Invalid action JSON from Python for %s: %s", state.vehicleId.c_str(), error.what());
    }
}

