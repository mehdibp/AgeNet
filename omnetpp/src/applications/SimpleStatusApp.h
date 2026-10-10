#pragma once
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include <map>
#include <string>
#include <fstream>


namespace veins {

class SimpleStatusApp : public DemoBaseApplLayer {
    private:
        struct VehicleState {
            std::string vehicleId;
            Coord position;
            double speed;
            std::string roadId;
            simtime_t lastSeen; 
        };
        
        VehicleState _getVehicleState();                        // Read the current vehicle state from Veins / SUMO
        double _getDistance(double x1, double y1, double x2, double y2) const;      // Distance between two positions
        void _printLog(const std::string& prefix, const std::string& details, const VehicleState& state);   // Log

        int messageCounter = 0;
        std::map<int, VehicleState> neighbors;
        simtime_t neighborTimeout = SimTime(3, SIMTIME_S);

        void sendVehicleStatus();
        void updateNeighbors();
        void printNeighborTable();

        std::ofstream neighborCsv;
        void saveNeighborsToCSV();
        void openNeighborCSV();

        
    protected:
        void handleSelfMsg(cMessage* msg) override;
        void handlePositionUpdate(cObject* obj) override;
        
        void onWSM(BaseFrame1609_4* wsm) override;

    public:
        void initialize(int stage) override;
        void finish() override;

};

} // namespace veins

