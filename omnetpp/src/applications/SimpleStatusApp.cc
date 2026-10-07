#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "SimpleStatusApp.h"

using namespace veins;
using namespace omnetpp;

Define_Module(SimpleStatusApp);



// Initialize the application ----------------------------------------------------------------
void SimpleStatusApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        scheduleAt(simTime() + SimTime(1, SIMTIME_S), new cMessage("statusTimer"));
        scheduleAt(simTime() + SimTime(2, SIMTIME_S) + uniform(0, 1), new cMessage("sendTimer"));
    }
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
        EV_INFO
            << "Vehicle: "  << findHost()->getFullName()
            << " | time: "  << simTime()
            << " | position: (" << position.x << ", " << position.y << ")"
            << " | speed: " << speed << " m/s"
            << " | road: "  << roadId
            << endl;

        scheduleAt(simTime() + SimTime(1, SIMTIME_S), msg);
    }
    else if (msg->isName("sendTimer")) {
        TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();

        populateWSM(wsm);
        wsm->setSenderAddress(myId);
        wsm->setSerial(messageCounter++);
        wsm->setDemoData(findHost()->getFullName());

        // wsm->setRecipientAddress(LAddress::L2BROADCAST());  // Explicitly broadcast to all eligible receivers

        EV_INFO << "[TX] Vehicle: "
                << findHost()->getFullName()
                << " | time: " << simTime()
                << " | serial: " << wsm->getSerial()
                << endl;

        sendDown(wsm);

        scheduleAt(simTime() + SimTime(5, SIMTIME_S) + uniform(0.01, 0.5), msg);
    }
    else { DemoBaseApplLayer::handleSelfMsg(msg); }

}

// Handle received messages ------------------------------------------------------------------
void SimpleStatusApp::onWSM(BaseFrame1609_4* wsm) {
    TraCIDemo11pMessage* message = check_and_cast<TraCIDemo11pMessage*>(wsm);

    EV_INFO << "[RX] Vehicle: "
            << findHost()->getFullName()
            << " | time: " << simTime()
            << " | sender: " << message->getDemoData()
            << " | serial: " << message->getSerial()
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
