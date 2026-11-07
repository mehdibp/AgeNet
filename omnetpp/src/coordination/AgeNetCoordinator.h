#pragma once

#include <omnetpp.h>
#include <nlohmann/json.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>


namespace veins {

class AgeNetApp;
class PythonBridge;

class AgeNetCoordinator : public omnetpp::cSimpleModule {
    private:
        void connectToPython();
        void runCycle();

        omnetpp::cMessage* cycleTimer = nullptr;
        std::unique_ptr<PythonBridge> pythonBridge;

        std::map<std::string, AgeNetApp*> vehicles;           // ordered -> deterministic message order
        std::map<std::string, SectionProvider> sections;      // extra (non-vehicle) sections

        long step = 0;
        double cyclePeriod = 1.0;
        double broadcastJitter = 0.1;

    protected:
        void initialize() override;
        void handleMessage(omnetpp::cMessage* msg) override;
        void finish() override;

    public:
        // A "section" is one top-level list of the observation ("vehicles", "traffic_lights", ...).
        // The provider is called once per cycle and must return a JSON object {id: {...}, ...}.
        using SectionProvider = std::function<nlohmann::json()>;

        ~AgeNetCoordinator() override;

        void registerVehicle(const std::string& vehicleId, AgeNetApp* app);
        void unregisterVehicle(const std::string& vehicleId);
        void registerSection(const std::string& name, SectionProvider provider);
};

} // namespace veins
