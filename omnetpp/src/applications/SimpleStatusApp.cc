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

void SimpleStatusApp::handleSelfMsg(cMessage* msg) {
    EV_INFO
        << "Hello from "
        << findHost()->getFullName()
        << " at "
        << simTime()
        << endl;

    scheduleAt(simTime() + SimTime(1, SIMTIME_S), msg);
}

