package src.coordination;

// One instance per network. Owns the Python socket and runs the synchronous loop:
//   collect all vehicles -> ONE message to Python -> apply ALL actions -> everyone broadcasts.
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

    @class(veins::AgeNetCoordinator);
    // @display("i=block/network2");
}
