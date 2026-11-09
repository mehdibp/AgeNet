"""
AgeNet.bridge -- the "traci" of OMNeT++.

    from AgeNet import bridge

    bridge.start(port=5555)
    while bridge.simulationStep():
        for vid in bridge.vehicle.getIDList():
            x, y = bridge.vehicle.getPosition(vid)
            bridge.vehicle.setRadius(vid, 120.0)
    bridge.close()

Module-level names below belong to ONE default connection (like traci). If you ever need
two connections in one process (or isolated tests), create `Bridge()` objects yourself.

This package must stay independent of the rest of AgeNet (it imports only the standard
library and numpy), so environments/ can import it without circular imports.
"""

from .core import Bridge
from .protocol import PROTOCOL_VERSION, BridgeError, ProtocolError, UnknownEntityError

_default = Bridge()

# lifecycle
start          = _default.start
simulationStep = _default.simulationStep
close          = _default.close
isConnected    = _default.isConnected

# domains
vehicle        = _default.vehicle
simulation     = _default.simulation
trafficlight   = _default.trafficlight
section        = _default.section
