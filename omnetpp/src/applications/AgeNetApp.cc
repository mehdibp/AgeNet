#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

#include "AgeNetApp.h"
#include "bridge/AgentProtocol.h"
#include "bridge/PythonBridge.h"
#include "logging/ConsoleLog.h"
#include "messages/VehicleStatusMessage_m.h"

#include <iomanip>
#include <cmath>
#include <stdexcept>


using namespace veins;
using namespace omnetpp;

Define_Module(AgeNetApp);


namespace {
    constexpr double FirstEventDelay  = 2.0;
    constexpr double ShowStatusPeriod = 1.0;
    constexpr double SendPeriod       = 1.0;
    constexpr double NeighborPeriod   = 1.0;
    constexpr double NeighborTimeout  = 3.0;
    constexpr double PythonPeriod     = 1.0;
}


// Initialize the application ----------------------------------------------------------------
void AgeNetApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        csvLogger.open(findHost()->getFullName());
        
        scheduleAt(simTime() + SimTime(FirstEventDelay, SIMTIME_S) + uniform(0, 1), new cMessage("statusTimer"));
        scheduleAt(simTime() + SimTime(FirstEventDelay, SIMTIME_S) + uniform(0, 1), new cMessage("sendTimer"));
        scheduleAt(simTime() + SimTime(FirstEventDelay, SIMTIME_S) + uniform(0, 1), new cMessage("neighborTimer"));

        if (par("pythonBridgeEnabled").boolValue()) connectToPython();

    }
}

// Finish the application --------------------------------------------------------------------
AgeNetApp::~AgeNetApp() {
    pythonBridge.reset();
    if (csvLogger.isOpen()) csvLogger.Close();
    DemoBaseApplLayer::finish();
}


// -------------------------------------------------------------------------------------------
// Read the current vehicle state ------------------------------------------------------------
VehicleState AgeNetApp::getVehicleState() {
    TraCIMobility* mobility = TraCIMobilityAccess().get(findHost());
    VehicleState state;

    state.vehicleId = findHost()->getFullName();
    state.position  = mobility->getPositionAt(simTime());
    state.speed     = mobility->getSpeed();
    state.roadId    = mobility->getRoadId();

    return state;
}


// -------------------------------------------------------------------------------------------
// Handle self-messages ----------------------------------------------------------------------
void AgeNetApp::handleSelfMsg(cMessage* msg) {

    if (msg->isName("statusTimer")) {
        // VehicleState state = getVehicleState();
        // console_log::transmit(state, "VEHICLE");
        // scheduleAt(simTime() + SimTime(ShowStatusPeriod, SIMTIME_S), msg);
    }
    else if (msg->isName("sendTimer")) {
        sendVehicleStatus();
        scheduleAt(simTime() + SimTime(SendPeriod, SIMTIME_S) + uniform(0.01, 0.5), msg);
    }
    else if (msg->isName("neighborTimer")) {
        updateNeighbors();
        scheduleAt(simTime() + SimTime(NeighborPeriod, SIMTIME_S), msg);
    }
    else if (msg->isName("pythonTimer")) {
        exchangeWithPython();
        scheduleAt(simTime() + SimTime(PythonPeriod, SIMTIME_S), msg);
    }
    else { DemoBaseApplLayer::handleSelfMsg(msg); }

}

// Handle received messages ------------------------------------------------------------------
void AgeNetApp::onWSM(BaseFrame1609_4* wsm) {
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

    console_log::received(findHost()->getFullName(), neighbor.vehicleId, neighbors.size());
}

// Handle Updating Position ------------------------------------------------------------------
void AgeNetApp::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // VehicleState state = getVehicleState();
    // console_log::transmit(state, "MOVEMENT");
}


// -------------------------------------------------------------------------------------------
// Send our vehicle status -------------------------------------------------------------------
void AgeNetApp::sendVehicleStatus() {
    const VehicleState state = getVehicleState();

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

    console_log::transmit(state, message->getSerial());
    sendDown(message);
}

// Remove neighbors that have timed out ------------------------------------------------------
void AgeNetApp::updateNeighbors() {
    for (auto it=neighbors.begin(); it!=neighbors.end();) {
        if (simTime() - it->second.lastSeen > SimTime(NeighborTimeout, SIMTIME_S)) {
            console_log::neighborLost(findHost()->getFullName(), it->second);

            it = neighbors.erase(it);
        }
        else {
            ++it;
        }
    }
    
    const VehicleState state = getVehicleState();
    console_log::neighborTable(state, neighbors);
    csvLogger.write(state, neighbors, simTime());
}

// Python bridge -----------------------------------------------------------------------------
void AgeNetApp::connectToPython() {
    const std::string host = par("pythonHost").stdstringValue();
    const int port         = par("pythonPort").intValue();

    pythonBridge = std::make_unique<PythonBridge>();
    try { pythonBridge->connectToServer(host, port); }
    catch (const std::exception& error) {
        pythonBridge.reset();
        throw cRuntimeError("Python bridge connection failed for %s: %s", findHost()->getFullName(), error.what());
    }

    EV_INFO << "[PYTHON] Connected for vehicle " << findHost()->getFullName() << " to " << host << ":" << port << endl;
    scheduleAt(simTime() + SimTime(FirstEventDelay, SIMTIME_S) + uniform(0, 1), new cMessage("pythonTimer"));
}

// Exchange the current observation and receive a test action from Python --------------------
void AgeNetApp::exchangeWithPython() {
    if (!pythonBridge || !pythonBridge->isConnected()) {
        throw cRuntimeError("Python bridge is not connected for %s", findHost()->getFullName());
    }

    const VehicleState state = getVehicleState();

    try {
        const auto reply  = pythonBridge->exchange(agent_protocol::buildObservation(state, neighbors, simTime()));
        const auto action = agent_protocol::parseAction(reply, state.vehicleId);
        
        console_log::pythonAction(state, action.txPower, neighbors.size());

        // Integration stage only: the action is validated and logged, but the radio
        // transmit power is intentionally not modified yet.
    }
    catch (const std::exception& error) {
        throw cRuntimeError("Python bridge exchange failed for %s: %s", state.vehicleId.c_str(), error.what());
    }
}
