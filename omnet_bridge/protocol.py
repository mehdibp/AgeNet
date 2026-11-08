# ============================================================
# Wire protocol between the C++ AgeNetCoordinator and Python
# (one JSON object per line). The C++ twin of this file is
# omnetpp/src/bridge/AgentProtocol.{h,cc}: change both together.
#
#   OMNeT -> Python : {"type":"hello", "protocol":1, "playground":[X,Y], "cycle_period":1.0}   (once)
#   Python -> OMNeT : {"type":"hello_ack", "protocol":1}                                       (once)
#   OMNeT -> Python : {"type":"observe", "step":n, "sim_time":t, "vehicles":{...}, <section>:{...}}
#   Python -> OMNeT : {"type":"action",  "step":n, "sim_time":t, "actions":{vid:{"txPower_mW":x}}}
# ============================================================

from typing import Any, Dict, Tuple


PROTOCOL_VERSION = 1

VEHICLES = "vehicles"
ENVELOPE_KEYS = frozenset({"type", "step", "sim_time"})   # observation keys that are NOT entity sections


# -------------------------------------------------------------------------------------------
class BridgeError(RuntimeError):
    """ Base class of every error raised by the bridge. """

class ProtocolError(BridgeError):
    """ The other side sent something that is not part of the protocol. """

class UnknownEntityError(BridgeError, LookupError):
    """ An id that is not in the current observation (e.g. a vehicle that has already left). """


# -------------------------------------------------------------------------------------------
def parse_hello(message: Dict[str, Any]) -> Tuple[float, float, float]:
    """ Validate a hello message. Returns (playground_x, playground_y, cycle_period). """
    if message.get("type") != "hello":
        raise ProtocolError(f"expected hello, got: {message}")

    if message.get("protocol") != PROTOCOL_VERSION:
        raise ProtocolError(f"protocol version mismatch: python={PROTOCOL_VERSION}, omnet={message.get('protocol')}")

    try:
        width, height = (float(v) for v in message["playground"])
        cycle_period  = float(message.get("cycle_period", 1.0))
    except (KeyError, TypeError, ValueError) as error:
        raise ProtocolError(f"malformed hello: {error!r}") from error

    return width, height, cycle_period


def parse_observation(message: Dict[str, Any]) -> Tuple[int, float]:
    """ Validate an observation message. Returns (step, sim_time). """
    if message.get("type") != "observe":
        raise ProtocolError(f"expected observe, got: {message}")

    try:
        step     = int(message["step"])
        sim_time = float(message["sim_time"])

        vehicles = message.get(VEHICLES, {})
        if not isinstance(vehicles, dict): raise TypeError("'vehicles' must be an object")
        for vehicle_id, data in vehicles.items():
            if len(data["position"]) != 2: raise ValueError(f"{vehicle_id}: position must be [x, y]")
            float(data["speed"])

        for name, content in message.items():
            if name in ENVELOPE_KEYS or name == VEHICLES: continue
            if not isinstance(content, dict): raise TypeError(f"section '{name}' must be an object {{id: {{...}}}}")

    except (KeyError, TypeError, ValueError) as error:
        raise ProtocolError(f"malformed observation: {error!r}") from error

    return step, sim_time


# -------------------------------------------------------------------------------------------
def hello_ack() -> Dict[str, Any]:
    return {"type": "hello_ack", "protocol": PROTOCOL_VERSION}


def error_message(text: str) -> Dict[str, Any]:
    return {"type": "error", "message": text}


def build_action(step: int, sim_time: float, tx_powers: Dict[str, float]) -> Dict[str, Any]:
    return {
        "type"    : "action",
        "step"    : step,                   # echoed back so OMNeT++ can detect a desync
        "sim_time": sim_time,
        "actions" : { vehicle_id: {"txPower_mW": power} for vehicle_id, power in tx_powers.items() }
    }
