#pragma once
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"


namespace veins {

class SimpleStatusApp : public DemoBaseApplLayer {
    public:
        void initialize(int stage) override;

    protected:
        void handleSelfMsg(cMessage* msg) override;
};

} // namespace veins
