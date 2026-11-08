# ============================================================
# Bridge: the "traci" of OMNeT++.
# Owns the connection and the snapshot, and exposes the lifecycle
# (start / simulationStep / close) plus the domains (vehicle, simulation, ...).
# ============================================================

from .connection import Connection
from .domains import State, VehicleDomain, SimulationDomain, SectionDomain
from .protocol import (
    BridgeError, ProtocolError,
    parse_hello, parse_observation, hello_ack, error_message, build_action,
)


class Bridge:
    """
    Usage (same rhythm as traci):

        bridge.start(port=5555)             # ~ traci.start(cmd): wait for OMNeT++, handshake
        while bridge.simulationStep():      # ~ traci.simulationStep(): send queued actions, wait for next data
            for vid in bridge.vehicle.getIDList():
                bridge.vehicle.getPosition(vid)
                bridge.vehicle.setTxPower(vid, 17.5)
        bridge.close()                      # ~ traci.close()

    simulationStep() == "send the actions queued for the observation I currently hold, then
    block until the next observation". So the first call only receives.
    """

    # ---------------------------------------------------------------------------------------
    def __init__(self):
        self._state = State()
        self._connection: Connection | None = None
        self._awaiting_reply = False            # True while OMNeT++ is waiting for our actions
        self._finished       = False            # True once OMNeT++ hung up (end of its run)

        self.vehicle      = VehicleDomain   (self._state)
        self.simulation   = SimulationDomain(self._state)
        self.trafficlight = SectionDomain   (self._state, "traffic_lights")

    # ---------------------------------------------------------------------------------------
    def section(self, name: str) -> SectionDomain:
        """ A domain for any other section of the observation (see SectionDomain). """
        return SectionDomain(self._state, name)

    def isConnected(self) -> bool:
        return self._connection is not None and not self._finished

    # lifecycle -----------------------------------------------------------------------------
    def start(self, host: str="0.0.0.0", port: int=5555, acceptTimeout: float | None=None, readTimeout: float | None=None):
        """ Wait for OMNeT++ to connect, then do the hello/hello_ack handshake (static facts). """
        if self._connection is not None: raise BridgeError("Already started; call close() first")

        self._connection = Connection(host, port, acceptTimeout, readTimeout)
        try:
            self._connection.open()
            self._handshake()

        except BaseException:
            self.close()
            raise

    def simulationStep(self) -> bool:
        """
        Advance the simulation by one cycle.

        Returns:
            True  : a new observation arrived (all getters now return the new data)
            False : OMNeT++ finished/disconnected. Nothing changes any more; stop your loop.
        """
        if self._connection is None: raise BridgeError("start() must be called first")
        if self._finished: return False

        if self._awaiting_reply:
            sent = self._send_actions()
            if not sent: return self._finish()

        return self._receive_observation()

    def close(self):
        """ Close the connection. (If OMNeT++ is still running it will stop with a socket error.) """
        if self._connection is not None: self._connection.close()

        self._connection = None
        self._awaiting_reply = False
        self._finished = False
        self._state.reset()

    # internal ------------------------------------------------------------------------------
    def _handshake(self):
        hello = self._connection.receive()
        if hello is None: raise ProtocolError("OMNeT++ disconnected before sending hello")

        try:
            width, height, cycle_period = parse_hello(hello)
        except ProtocolError as error:
            self._connection.send(error_message(str(error)))    # so the C++ side prints WHY
            raise

        self._state.bounds  = (0.0, 0.0, width, height)
        self._state.delta_t = cycle_period
        self._connection.send(hello_ack())

    def _receive_observation(self) -> bool:
        observation = self._connection.receive()
        if observation is None: return self._finish()

        step, sim_time = parse_observation(observation)
        self._state.update(observation, step, sim_time)
        self._awaiting_reply = True
        return True

    def _send_actions(self) -> bool:
        message = build_action(self._state.step, self._state.time, self._state.pending_actions)
        self._state.pending_actions = {}
        self._awaiting_reply = False
        return self._connection.send(message)

    def _finish(self) -> bool:
        self._finished = True
        self._awaiting_reply = False
        return False
