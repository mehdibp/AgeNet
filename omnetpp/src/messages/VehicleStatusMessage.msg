import veins.modules.messages.BaseFrame1609_4;

namespace veins;

packet VehicleStatusMessage extends BaseFrame1609_4 {
    int senderId;
    int serial;
    string vehicleId;

    double posX;
    double posY;
    double speed;

    simtime_t creationTime;
}
