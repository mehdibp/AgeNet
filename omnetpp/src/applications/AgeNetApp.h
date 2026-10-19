#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "logging/NeighborCsvLogger.h"
#include "VehicleState.h"

#include <map>
#include <memory>
#include <string>
#include <fstream>


namespace veins {

class PythonBridge;

class AgeNetApp : public DemoBaseApplLayer {
    private:        
        VehicleState getVehicleState();             // Read the current vehicle state from Veins / SUMO
        void sendVehicleStatus();                   // Broadcast my state
        void updateNeighbors();
        void connectToPython();
        void exchangeWithPython();                  // Send observation, receive + validate action

        int messageCounter = 0;
        NeighborCsvLogger csvLogger;
        std::map<int, VehicleState> neighbors;
        std::unique_ptr<PythonBridge> pythonBridge;

    protected:
        void handlePositionUpdate(cObject* obj) override;
        void handleSelfMsg(cMessage* msg) override;
        void onWSM(BaseFrame1609_4* wsm) override;

    public:
        ~AgeNetApp() override;
        void initialize(int stage) override;

};

} // namespace veins

