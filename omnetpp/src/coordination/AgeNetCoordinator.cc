#include "AgeNetCoordinator.h"

#include "applications/AgeNetApp.h"
#include "bridge/AgentProtocol.h"
#include "bridge/PythonBridge.h"

#include <set>
#include <stdexcept>

using namespace veins;
using namespace omnetpp;

Define_Module(AgeNetCoordinator);


// Lifecycle ---------------------------------------------------------------------------------
void AgeNetCoordinator::initialize() {
    cyclePeriod     = par("cyclePeriod").doubleValue();
    broadcastJitter = par("broadcastJitter").doubleValue();

    if (par("pythonBridgeEnabled").boolValue()) connectToPython();

    cycleTimer = new cMessage("cycleTimer");
    scheduleAt(SimTime(par("firstCycleTime").doubleValue()), cycleTimer);
}

AgeNetCoordinator::~AgeNetCoordinator() {
    cancelAndDelete(cycleTimer);
}

void AgeNetCoordinator::finish() {
    pythonBridge.reset();
}

void AgeNetCoordinator::handleMessage(cMessage* msg) {
    if (msg == cycleTimer) {
        runCycle();
        scheduleAt(simTime() + SimTime(cyclePeriod), cycleTimer);
    }
    else { throw cRuntimeError("AgeNetCoordinator got an unexpected message: %s", msg->getName()); }
}


// Registry ----------------------------------------------------------------------------------
void AgeNetCoordinator::registerVehicle(const std::string& vehicleId, AgeNetApp* app) {
    if (!vehicles.emplace(vehicleId, app).second) {
        throw cRuntimeError("Vehicle '%s' is already registered in the coordinator", vehicleId.c_str());
    }
}

void AgeNetCoordinator::unregisterVehicle(const std::string& vehicleId) {
    vehicles.erase(vehicleId);
}

void AgeNetCoordinator::registerSection(const std::string& name, SectionProvider provider) {
    sections[name] = std::move(provider);
}


// Python bridge -----------------------------------------------------------------------------
void AgeNetCoordinator::connectToPython() {
    const std::string host = par("pythonHost").stdstringValue();
    const int port         = par("pythonPort").intValue();
    const double timeout   = par("socketTimeout").doubleValue();

    pythonBridge = std::make_unique<PythonBridge>();
    try { pythonBridge->connectToServer(host, port, timeout); }
    catch (const std::exception& error) {
        pythonBridge.reset();
        throw cRuntimeError("Python bridge connection failed: %s", error.what());
    }

    EV_INFO << "[PYTHON] Coordinator connected to " << host << ":" << port << endl;
}


// One synchronous cycle ---------------------------------------------------------------------
void AgeNetCoordinator::runCycle() {
    if (vehicles.empty()) return;

    ++step;
    const simtime_t now = simTime();

    // 1) Every vehicle refreshes its neighbor table (timeouts) and we snapshot ALL of them.
    nlohmann::json allSections = nlohmann::json::object();
    nlohmann::json vehicleSection = nlohmann::json::object();
    std::set<std::string> knownVehicles;

    for (auto& [vehicleId, app] : vehicles) {
        app->updateNeighbors();
        vehicleSection[vehicleId] = agent_protocol::buildVehicleEntry(app->getVehicleState(), app->getNeighbors(), now);
        knownVehicles.insert(vehicleId);
    }
    allSections["vehicles"] = std::move(vehicleSection);

    for (auto& [name, provider] : sections) allSections[name] = provider();

    // 2) ONE exchange with Python, ONE list of decisions back.
    if (pythonBridge) {
        try {
            const auto reply   = pythonBridge->exchange(agent_protocol::buildObservation(step, now, allSections));
            const auto actions = agent_protocol::parseActions(reply, step, knownVehicles);

            // 3) Apply every decision before anyone transmits.
            for (const auto& [vehicleId, action] : actions) vehicles.at(vehicleId)->applyAction(action.txPower);
        }
        catch (const std::exception& error) {
            throw cRuntimeError("Python bridge exchange failed at step %ld: %s", step, error.what());
        }
    }

    // 4) Everyone broadcasts with its (new) power; small jitter so frames don't all collide.
    for (auto& [vehicleId, app] : vehicles) {
        app->scheduleBroadcast(SimTime(uniform(0, broadcastJitter)));
    }
}
