# ============================================================
# TCP transport (Python is the server, OMNeT++ connects to it)
# Knows NOTHING about vehicles or the protocol: it only moves JSON lines.
# ============================================================

import json
import socket
from typing import Any, Dict

from .protocol import ProtocolError


class Connection:
    # ---------------------------------------------------------------------------------------
    def __init__(self, host: str="0.0.0.0", port: int=5555,
                 accept_timeout: float | None=None, read_timeout: float | None=None):
        """
        Args:
            host            : address to listen on
            port            : TCP port (must match *.agenetCoordinator.pythonPort in omnetpp.ini)
            accept_timeout  : seconds to wait for OMNeT++ to connect (None = forever)
            read_timeout    : seconds to wait for each message from OMNeT++ (None = forever)
        """
        self.host = host
        self.port = port
        self.accept_timeout = accept_timeout
        self.read_timeout   = read_timeout

        self._server: socket.socket | None = None
        self._conn  : socket.socket | None = None
        self._file = None

    # ---------------------------------------------------------------------------------------
    def open(self):
        """ Listen and block until OMNeT++ connects. """
        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server.bind((self.host, self.port))
        self._server.listen(1)
        self._server.settimeout(self.accept_timeout)

        print(f"[BRIDGE] waiting for OMNeT++ on {self.host}:{self.port} ...")
        self._conn, address = self._server.accept()
        self._conn.settimeout(self.read_timeout)
        self._conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self._file = self._conn.makefile("rwb")
        print(f"[BRIDGE] connected: {address}")

    # ---------------------------------------------------------------------------------------
    def send(self, message: Dict[str, Any]) -> bool:
        """ Send one message. Returns False if the peer is gone. """
        data = (json.dumps(message, separators=(",", ":")) + "\n").encode()
        try:
            self._file.write(data)
            self._file.flush()
            return True
        except (BrokenPipeError, ConnectionError):
            return False

    # ---------------------------------------------------------------------------------------
    def receive(self) -> Dict[str, Any] | None:
        """ One message, or None when the peer closed the connection. """
        try:
            line = self._file.readline()
        except ConnectionError:
            return None

        if not line: return None

        try: return json.loads(line)
        except json.JSONDecodeError as error: raise ProtocolError(f"invalid JSON from OMNeT++: {error}") from error

    # ---------------------------------------------------------------------------------------
    def close(self):
        for resource in (self._file, self._conn, self._server):
            try:
                if resource is not None: resource.close()
            except OSError:
                pass

        self._file = self._conn = self._server = None
