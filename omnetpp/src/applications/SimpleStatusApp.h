#pragma once
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include <map>
#include <string>


namespace veins {

class SimpleStatusApp : public DemoBaseApplLayer {
    private:
        int messageCounter = 0;

        struct Neighbor {
            int senderId;
            // int serial;
            std::string vehicleId;
            double posX;
            double posY;
            double speed;
            simtime_t lastSeen; 
        };
        
        std::map<int, Neighbor> neighbors;
        simtime_t neighborTimeout = SimTime(3, SIMTIME_S);

        void sendVehicleStatus();
        void updateNeighbors();
        void saveNeighborsToCSV();
        
    protected:
        void handleSelfMsg(cMessage* msg) override;
        void handlePositionUpdate(cObject* obj) override;
        
        void onWSM(BaseFrame1609_4* wsm) override;

    public:
        void initialize(int stage) override;
        void finish() override;

};

} // namespace veins
