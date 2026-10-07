#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "messages/VehicleStatusMessage_m.h"
#include "SimpleStatusApp.h"

using namespace veins;
using namespace omnetpp;

Define_Module(SimpleStatusApp);


// -------------------------------------------------------------------------------------------
// Initialize the application ----------------------------------------------------------------
void SimpleStatusApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        scheduleAt(simTime() + SimTime(1, SIMTIME_S), new cMessage("statusTimer"));
        scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), new cMessage("sendTimer"));
        scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), new cMessage("neighborTimer"));
    }
}

// Finish the application --------------------------------------------------------------------
void SimpleStatusApp::finish() {
    DemoBaseApplLayer::finish();
}


// Handle self-messages ----------------------------------------------------------------------
void SimpleStatusApp::handleSelfMsg(cMessage* msg) {

    if (msg->isName("statusTimer")) {
        TraCIMobility* mobility = TraCIMobilityAccess().get(findHost());    // Get the mobility module of this vehicle.

        // Read the current vehicle state.
        Coord position      = mobility->getPositionAt(simTime());
        double speed        = mobility->getSpeed();
        std::string roadId  = mobility->getRoadId();

        // Print the vehicle state.
        // EV_INFO
        //     << "Vehicle: "  << findHost()->getFullName()
        //     << " | time: "  << simTime()
        //     << " | position: (" << position.x << ", " << position.y << ")"
        //     << " | speed: " << speed << " m/s"
        //     << " | road: "  << roadId
        //     << endl;

        // scheduleAt(simTime() + SimTime(1, SIMTIME_S), msg);
    }
    else if (msg->isName("sendTimer")) {
        sendVehicleStatus();
        scheduleAt(simTime() + SimTime(1, SIMTIME_S) + uniform(0.01, 0.5), msg);
    }
    else if (msg->isName("neighborTimer")) {
        updateNeighbors();
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

    Neighbor neighbor;

    neighbor.senderId   = senderId;
    neighbor.vehicleId  = message->getVehicleId();
    neighbor.posX       = message->getPosX();
    neighbor.posY       = message->getPosY();
    neighbor.speed      = message->getSpeed();
    neighbor.lastSeen   = simTime();

    neighbors[senderId] = neighbor;

    EV_INFO
        << "[RX] Vehicle: "  << findHost()->getFullName()
        << " received from " << neighbor.vehicleId
        << " | position: ("  << neighbor.posX << ", " << neighbor.posY << ")"
        << " | speed: "      << neighbor.speed
        << " | neighbors: "  << neighbors.size()
        << endl;
}

// Handle Updating Position ------------------------------------------------------------------
void SimpleStatusApp::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);

    TraCIMobility* mobility = TraCIMobilityAccess().get(findHost());

    Coord position = mobility->getPositionAt(simTime());
    double speed        = mobility->getSpeed();
    std::string roadId  = mobility->getRoadId();

    // EV_INFO
    //     << "[MOVEMENT EVENT] "
    //     << "vehicle=" << findHost()->getFullName()
    //     << " time=" << simTime()
    //     << " position=(" << position.x << ", " << position.y << ")"
    //     << " speed=" << speed << " m/s"
    //     << " road=" << roadId
    //     << endl;
}


// -------------------------------------------------------------------------------------------
void SimpleStatusApp::sendVehicleStatus() {
    TraCIMobility* mobility = TraCIMobilityAccess().get(findHost());
    Coord position = mobility->getPositionAt(simTime());

    VehicleStatusMessage* message = new VehicleStatusMessage();
    populateWSM(message);

    message->setSenderId(myId);
    message->setVehicleId(findHost()->getFullName());
    message->setPosX(position.x);
    message->setPosY(position.y);
    message->setSpeed(mobility->getSpeed());
    message->setCreationTime(simTime());
    message->setSerial(messageCounter++);

    message->setRecipientAddress(LAddress::L2BROADCAST());      // Broadcast

    EV_INFO
        << "[TX] Vehicle: " << findHost()->getFullName()
        << " | serial: " << message->getSerial()
        << " | position: (" << position.x << ", " << position.y << ")"
        << " | speed: " << mobility->getSpeed()
        << endl;

    sendDown(message);
}

// -------------------------------------------------------------------------------------------
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

