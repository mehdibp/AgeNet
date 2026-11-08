# ============================================================
# OMNeT++ / Veins physical environment adapter
# ============================================================

import numpy as np
from typing import List, Tuple

from ... import omnet_bridge as bridge           # networking lives in VaNet/bridge (the "traci" of OMNeT++)
from .base import Environment, Entity


class OmnetEnvironment(Environment):
    """
    Adapter layer between OMNeT++ (through AgeNet.bridge) and the multi-agent RL system.
    Same shape as SumoEnvironment: start() / step() / close() + cached entities.

    Note: step() returns False when the OMNeT++ run is over (sim-time-limit reached),
    so write the loop as  `while environment.step(): ...`
    """

    # ---------------------------------------------------------------------------------------
    def __init__(self, host: str="0.0.0.0", port: int=5555, accept_timeout: float|None=None, read_timeout: float|None=None):
        """
        Args:
            host            : address to listen on
            port            : TCP port (must match *.agenetCoordinator.pythonPort in omnetpp.ini)
            accept_timeout  : seconds to wait for OMNeT++ to connect (None = forever)
            read_timeout    : seconds to wait for the next observation (None = forever)
        """
        super().__init__()

        self.host = host
        self.port = port
        self.accept_timeout = accept_timeout
        self.read_timeout   = read_timeout

        self._started = False
        self._net_bounds: Tuple[float, float, float, float] | None = None


    # OMNeT lifecycle -----------------------------------------------------------------------
    def start(self):
        """ Wait for OMNeT++, handshake, receive the first observation. """
        if self._started: return

        bridge.start(self.host, self.port, self.accept_timeout, self.read_timeout)
        self._started = True

        self._load_network_bounds()
        self.step()

    def close(self):
        """ Close the connection safely. """
        if self._started:
            bridge.close()
            self._started = False

        super().close()                         # clears self._entities

    def reset(self):
        """ Wait for a NEW OMNeT++ run (python can't restart an OMNeT++ process on its own). """
        self.close()
        self.start()

    def step(self) -> bool:
        """
        Advance the simulation by one cycle: send the queued actions, wait for new data.
        Returns False when OMNeT++ has finished.
        """
        running = bridge.simulationStep()
        if not running: return False

        self._cache_step_data()
        return True


    # geometry ------------------------------------------------------------------------------
    def get_bounds(self) -> Tuple[float, float, float, float]:
        """ Return (xmin, ymin, xmax, ymax) of the OMNeT++ playground (same coordinates as positions). """
        return self._net_bounds

    def get_density(self) -> float:
        """ Vehicle density per unit area. """
        if self._net_bounds is None: return 0.0

        xmin, ymin, xmax, ymax = self._net_bounds
        area = max((xmax - xmin) * (ymax - ymin), 1e-6)
        return len(self._entities) / area


    # extra queries (beyond the Environment API) --------------------------------------------
    def get_departed_ids(self) -> List[str]: 
        return bridge.simulation.getDepartedIDList()
    def get_arrived_ids (self) -> List[str]: 
        return bridge.simulation.getArrivedIDList()


    # internal ------------------------------------------------------------------------------
    def _cache_step_data(self):
        """ Cache vehicle positions and ID. """
        self._time  = bridge.simulation.getTime()
        vehicle_ids = bridge.vehicle.getIDList()

        self._entities.clear()
        for vid in vehicle_ids:
            pos    = bridge.vehicle.getPosition(vid)
            speed  = bridge.vehicle.getSpeed   (vid)
            roadId = bridge.vehicle.getRoadID  (vid)
            self._entities[vid] = Entity(np.array(pos, dtype=np.float32), speed, None, roadId)

    def _load_network_bounds(self):
        """ The playground that OMNeT++ announced in its hello message. """
        (xmin, ymin), (xmax, ymax) = bridge.simulation.getNetBoundary()
        self._net_bounds = (xmin, ymin, xmax, ymax)
