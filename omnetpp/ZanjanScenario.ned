import org.car2x.veins.nodes.Scenario;
import org.car2x.veins.nodes.TrafficLight;

// Same idea as RSUExampleScenario, but without any RSU.
// Vehicles (node[*]) are NOT declared here: the TraCI manager creates/removes them
// dynamically as SUMO inserts/removes cars.
network ZanjanScenario extends Scenario
{
    submodules:
        tls[0]: TrafficLight { }
}
