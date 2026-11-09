package src.coordination;

// One instance per network. Owns the Python socket and runs the synchronous loop:
//   collect all vehicles -> ONE message to Python -> apply ALL actions -> everyone broadcasts.
// Python decides RADII; this module converts them to transmit power (see radio/TxPowerModel.h).
simple AgeNetCoordinator {
    parameters:
        bool   pythonBridgeEnabled = default(false);
        string pythonHost = default("127.0.0.1");
        int    pythonPort = default(5555);
        double socketTimeout @unit(s) = default(30s);

        // Keep firstCycleTime off the TraCI step grid (TraCI moves cars at integer seconds
        // when updateInterval = 1s), so every vehicle has already moved when the cycle runs.
        double firstCycleTime  @unit(s) = default(2.1s);
        double cyclePeriod     @unit(s) = default(1s);
        double broadcastJitter @unit(s) = default(0.1s);   // desync beacons to avoid collisions

        // ---- radius <-> power model (TxPowerModel) ----
        // carrierFrequency, pathLossAlpha and noiseFloor DUPLICATE values that live in config.xml
        // (centerFrequency, alpha) and omnetpp.ini (phy80211p.noiseFloor). Keep them in sync!
        double carrierFrequency @unit(Hz)  = default(5.89GHz);
        double pathLossAlpha               = default(2.0);
        double noiseFloor       @unit(dBm) = default(-98dBm);
        double minSinr          @unit(dB)  = default(8dB);      // 6 Mbps = QPSK 1/2; design margin
        double minTxPower       @unit(mW)  = default(0.0001mW); // lower clamp  -> smallest radius
        double maxTxPower       @unit(mW)  = default(100mW);    // upper clamp  -> largest radius

    @class(veins::AgeNetCoordinator);
    @display("p=-100,475;i=block/decap");
}
