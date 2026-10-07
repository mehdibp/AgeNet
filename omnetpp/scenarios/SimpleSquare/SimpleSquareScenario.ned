package scenarios.SimpleSquare;

import org.car2x.veins.nodes.Scenario;
import org.car2x.veins.nodes.TrafficLight;

// Same idea as RSUExampleScenario, but without any RSU.
// Vehicles (node[*]) are NOT declared here: the TraCI manager creates/removes them
// dynamically as SUMO inserts/removes cars.
network SimpleSquareScenario extends Scenario {
    // @display("bgb=8840, 12380");

    submodules:
        tls[0]: TrafficLight { }
}

