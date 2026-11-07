package src.applications;
import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple AgeNetApp extends DemoBaseApplLayer {
    parameters:
        bool enableConsoleLog = default(true);

    @class(veins::AgeNetApp);
}
