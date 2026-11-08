# ============================================================
# traci-like "domains": bridge.vehicle, bridge.simulation, bridge.trafficlight, ...
#
# IMPORTANT: none of these getters touches the network. They only read the snapshot that the
# last simulationStep() cached in `State`. (Exactly one round trip per step, whatever the
# number of getters you call -- unlike real TraCI, where every call is a request.)
# ============================================================

import numpy as np
from typing import Any, Dict, List, Tuple

from .protocol import BridgeError, UnknownEntityError, ENVELOPE_KEYS, VEHICLES


# -------------------------------------------------------------------------------------------
class State:
    """ Everything the last observation said, plus the actions queued for the next step. """

    def __init__(self):
        self.reset()

    def reset(self):
        self.step     : int   = 0
        self.time     : float = 0.0
        self.delta_t  : float = 1.0
        self.bounds   : Tuple[float, float, float, float] | None = None   # (xmin, ymin, xmax, ymax)

        self.vehicles : Dict[str, Dict[str, Any]] = {}         # vid -> raw entry of the observation
        self.sections : Dict[str, Dict[str, Any]] = {}         # "traffic_lights" -> {id: {...}}
        self.departed : List[str] = []
        self.arrived  : List[str] = []

        self.pending_actions: Dict[str, float] = {}            # vid -> txPower_mW, sent by the next step

    def update(self, observation: Dict[str, Any], step: int, sim_time: float):
        previous = set(self.vehicles)

        self.step = step
        self.time = sim_time
        self.vehicles = dict(observation.get(VEHICLES, {}))
        self.sections = { name: content for name, content in observation.items()
                          if name not in ENVELOPE_KEYS and name != VEHICLES }

        self.departed = [vid for vid in self.vehicles if vid not in previous]
        self.arrived  = [vid for vid in previous if vid not in self.vehicles]


# -------------------------------------------------------------------------------------------
class VehicleDomain:
    """ Like traci.vehicle (read from the cache) + setTxPower (queued until the next step). """

    def __init__(self, state: State):
        self._state = state

    def _entry(self, vehicle_id: str) -> Dict[str, Any]:
        try: return self._state.vehicles[vehicle_id]
        except KeyError: raise UnknownEntityError(f"Unknown vehicle '{vehicle_id}' (not in the current observation)") from None

    # reading -------------------------------------------------------------------------------
    def getIDList(self) -> List[str]:
        return list(self._state.vehicles)

    def getIDCount(self) -> int:
        return len(self._state.vehicles)

    def getPosition(self, vehicle_id: str) -> Tuple[float, float]:
        x, y = self._entry(vehicle_id)["position"]
        return float(x), float(y)

    def getSpeed(self, vehicle_id: str) -> float:
        return float(self._entry(vehicle_id)["speed"])

    def getRoadID(self, vehicle_id: str) -> str | None:
        return self._entry(vehicle_id).get("road_id")

    def getNeighbors(self, vehicle_id: str) -> List[Dict[str, Any]]:
        """
        NOT in traci: the neighbors OMNeT++ heard for this vehicle, as received
            [{"sender_id","vehicle_id","x","y","speed","distance","age","road_id"}, ...]
        """
        return self._entry(vehicle_id).get("neighbors", [])

    # writing -------------------------------------------------------------------------------
    def setTxPower(self, vehicle_id: str, power_mW: float):
        """ Queue a new transmit power. Unlike traci.vehicle.setX, it is SENT by the next simulationStep(). """
        self._entry(vehicle_id)
        if not np.isfinite(power_mW) or power_mW <= 0.0:
            raise ValueError(f"txPower must be a positive number of mW, got {power_mW}")

        self._state.pending_actions[vehicle_id] = float(power_mW)


# -------------------------------------------------------------------------------------------
class SimulationDomain:
    """ Like traci.simulation. """

    def __init__(self, state: State):
        self._state = state

    def getTime(self) -> float:
        return self._state.time

    def getStep(self) -> int:
        """ NOT in traci: the coordinator's cycle counter (same number that is echoed in the action). """
        return self._state.step

    def getDeltaT(self) -> float:
        return self._state.delta_t

    def getNetBoundary(self) -> Tuple[Tuple[float, float], Tuple[float, float]]:
        """ ((xmin, ymin), (xmax, ymax)) of the OMNeT++ playground, in the same coordinates as positions. """
        if self._state.bounds is None: raise BridgeError("Bounds are known only after start()")
        xmin, ymin, xmax, ymax = self._state.bounds
        return (xmin, ymin), (xmax, ymax)

    def getDepartedIDList(self) -> List[str]:
        """ Vehicles that are new in the last step. """
        return list(self._state.departed)

    def getArrivedIDList(self) -> List[str]:
        """ Vehicles that were in the previous step but are gone now. """
        return list(self._state.arrived)


# -------------------------------------------------------------------------------------------
class SectionDomain:
    """
    Generic domain for any other top-level list of the observation ("traffic_lights", ...).
    Its content is whatever the C++ side puts there: {id: {...}}.
    """

    def __init__(self, state: State, name: str):
        self._state = state
        self.name   = name

    def _section(self) -> Dict[str, Any]:
        return self._state.sections.get(self.name, {})

    def getIDList(self) -> List[str]:
        return list(self._section())

    def getIDCount(self) -> int:
        return len(self._section())

    def get(self, entity_id: str) -> Dict[str, Any]:
        try: return self._section()[entity_id]
        except KeyError: raise UnknownEntityError(f"Unknown id '{entity_id}' in section '{self.name}'") from None

    def getAll(self) -> Dict[str, Any]:
        return dict(self._section())
