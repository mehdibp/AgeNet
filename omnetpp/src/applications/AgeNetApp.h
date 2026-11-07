#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "logging/NeighborCsvLogger.h"
#include "VehicleState.h"

#include <map>
#include <string>
#include <fstream>


namespace veins {

class AgeNetCoordinator;

class AgeNetApp : public DemoBaseApplLayer {
    private:
        void sendBeacon();                      // Broadcast my state

        std::string vehicleId;
        int messageCounter = 0;
        double txPowerMw = 0.0;

        NeighborCsvLogger csvLogger;
        std::map<int, VehicleState> neighbors;

        AgeNetCoordinator* coordinator = nullptr;
        omnetpp::cMessage* broadcastTimer = nullptr;

    protected:
        void handleSelfMsg(cMessage* msg) override;
        void onWSM(BaseFrame1609_4* wsm) override;

    public:
        ~AgeNetApp() override;
        void initialize(int stage) override;
        void finish() override;

        // ---- Called by AgeNetCoordinator (this module never talks to Python itself) ----
        VehicleState getVehicleState();                         // current state from Veins/SUMO
        const std::map<int, VehicleState>& getNeighbors() const { return neighbors; }
        void updateNeighbors();                                 // drop timed-out neighbors + log
        void applyAction(double txPowerMw);                     // set radio power
        void scheduleBroadcast(omnetpp::simtime_t delay);       // broadcast my status after `delay`

};

} // namespace veins
