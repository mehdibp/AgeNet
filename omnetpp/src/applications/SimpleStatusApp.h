#pragma once
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include <map>
#include <memory>
#include <string>
#include <fstream>


namespace veins {

class PythonBridge;

class SimpleStatusApp : public DemoBaseApplLayer {
    private:
        struct VehicleState {
            std::string vehicleId;
            Coord position;
            double speed;
            std::string roadId;
            simtime_t lastSeen; 
        };
        
        VehicleState getVehicleState();                         // Read the current vehicle state from Veins / SUMO
        void printLog(const std::string& prefix, const std::string& details, const VehicleState& state);    // Log

        int messageCounter = 0;
        std::map<int, VehicleState> neighbors;
        simtime_t neighborTimeout = SimTime(3, SIMTIME_S);

        void sendVehicleStatus();
        void updateNeighbors();
        void printNeighborTable();

        std::ofstream neighborCsv;
        void saveNeighborsToCSV();
        void openNeighborCSV();


        // PythonBridge owns all TCP socket operations. 
        // SimpleStatusApp owns the simulation scheduling and prepares the vehicle observation.
        std::unique_ptr<PythonBridge> pythonBridge;
        cMessage* pythonTimer = nullptr;
        void exchangeWithPython();

        
    protected:
        void handleSelfMsg(cMessage* msg) override;
        void handlePositionUpdate(cObject* obj) override;
        
        void onWSM(BaseFrame1609_4* wsm) override;

    public:
        ~SimpleStatusApp() override;
        void initialize(int stage) override;
        void finish() override;

};

} // namespace veins

