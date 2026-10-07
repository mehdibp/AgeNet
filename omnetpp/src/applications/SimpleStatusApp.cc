#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "SimpleStatusApp.h"

using namespace veins;

Define_Module(SimpleStatusApp);


// Initialize the application ----------------------------------------------------------------
void SimpleStatusApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        scheduleAt(simTime() + SimTime(1, SIMTIME_S), new cMessage("statusTimer"));
    }
}

// Handle self-messages ----------------------------------------------------------------------
void SimpleStatusApp::handleSelfMsg(cMessage* msg) {
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

// Handle Updating Position ------------------------------------------------------------------
void SimpleStatusApp::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);
}
