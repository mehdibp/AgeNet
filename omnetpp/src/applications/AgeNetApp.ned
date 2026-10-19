package src.applications;
import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple AgeNetApp extends DemoBaseApplLayer {
    parameters:
        bool pythonBridgeEnabled = default(false);
        string pythonHost = default("127.0.0.1");
        int pythonPort = default(5555);

    @class(veins::AgeNetApp);
}
