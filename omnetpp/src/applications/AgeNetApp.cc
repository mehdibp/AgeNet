#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"

#include "AgeNetApp.h"
#include "coordination/AgeNetCoordinator.h"
#include "logging/ConsoleLog.h"
#include "messages/VehicleStatusMessage_m.h"

#include <iomanip>
#include <cmath>
#include <stdexcept>


using namespace veins;
using namespace omnetpp;

Define_Module(AgeNetApp);


namespace {
    constexpr double NeighborTimeout  = 3.0;
}


// Initialize the application ----------------------------------------------------------------
void AgeNetApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        console_log::setEnabled(par("enableConsoleLog").boolValue());

        vehicleId = findHost()->getFullName();
        csvLogger.open(vehicleId);
        broadcastTimer = new cMessage("broadcastTimer");

        txPowerMw = findHost()->getSubmodule("nic")->getSubmodule("mac1609_4")->par("txPower").doubleValueInUnit("mW");

        // The network must contain a submodule called "agenetCoordinator".
        coordinator = check_and_cast<AgeNetCoordinator*>(getModuleByPath("<root>.agenetCoordinator"));
        coordinator->registerVehicle(vehicleId, this);
    }
}

// Veins calls finish() before deleting a vehicle that left SUMO, and at the end of the run --
void AgeNetApp::finish() {
    if (coordinator) {
        coordinator->unregisterVehicle(vehicleId);
        coordinator = nullptr;
    }
    if (csvLogger.isOpen()) csvLogger.Close();
    DemoBaseApplLayer::finish();
}

// Finish the application --------------------------------------------------------------------
AgeNetApp::~AgeNetApp() {
    cancelAndDelete(broadcastTimer);
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

// Send our vehicle status -------------------------------------------------------------------
void AgeNetApp::sendBeacon() {
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

// Handle self-messages ----------------------------------------------------------------------
void AgeNetApp::handleSelfMsg(cMessage* msg) {
    if (msg == broadcastTimer) sendBeacon();
    else DemoBaseApplLayer::handleSelfMsg(msg);
}


// -------------------------------------------------------------------------------------------
// Called by the coordinator (Enter_Method makes cross-module calls legal) -------------------
void AgeNetApp::scheduleBroadcast(simtime_t delay) {
    Enter_Method("scheduleBroadcast");
    cancelEvent(broadcastTimer);
    scheduleAt(simTime() + delay, broadcastTimer);
}

void AgeNetApp::applyAction(double newTxPowerMw) {
    Enter_Method("applyAction");

    txPowerMw = newTxPowerMw;
    // mac->setTxPower(txPowerMw);     // takes effect on the NEXT frame sent
    check_and_cast<Mac1609_4*>(findHost()->getSubmodule("nic")->getSubmodule("mac1609_4"))->setTxPower(txPowerMw);

    console_log::pythonAction(getVehicleState(), txPowerMw, neighbors.size());
}

// Remove neighbors that have timed out ------------------------------------------------------
void AgeNetApp::updateNeighbors() {
    Enter_Method("updateNeighbors");

    for (auto it=neighbors.begin(); it!=neighbors.end();) {
        if (simTime() - it->second.lastSeen > SimTime(NeighborTimeout, SIMTIME_S)) {
            console_log::neighborLost(vehicleId, it->second);

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

