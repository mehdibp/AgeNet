import json
import socketserver
import threading


HOST = "0.0.0.0"
PORT = 5555

# In the future, shared AgeNet state can be protected by this lock.
agent_lock = threading.Lock()


class AgeNetHandler(socketserver.StreamRequestHandler):
    def handle(self):
        print(f"[CONNECT] {self.client_address}")

        try:
            while True:
                line = self.rfile.readline()
                if not line: break

                try:
                    observation = json.loads(line.decode("utf-8"))
                    if observation.get("type") != "observe":
                        raise ValueError("Expected an observe message")

                    vehicle_id = observation["vehicle_id"]
                    with agent_lock:
                        self.process_observation(observation)

                        # Temporary dummy action; no RL yet.
                        response = { "type": "action", "vehicle_id": vehicle_id, "txPower_mW": 17.5, }

                    encoded = (json.dumps(response, separators=(",", ":")) + "\n").encode("utf-8")
                    self.wfile.write(encoded)
                    self.wfile.flush()

                except (ValueError, KeyError, TypeError) as exc:
                    print(f"[INVALID MESSAGE] {exc}")

                    response = {"type": "error", "message": "invalid_message",}
                    self.wfile.write((json.dumps(response) + "\n").encode("utf-8"))
                    self.wfile.flush()

        finally:
            print(f"[DISCONNECT] {self.client_address}")

    @staticmethod
    def process_observation(obs):
        vehicle_id = obs["vehicle_id"]
        sim_time = obs["sim_time"]
        position = obs["position"]
        neighbors = obs["neighbors"]

        print(
            f"[OBS] t={sim_time:.1f}s "
            f"vehicle={vehicle_id} "
            f"pos=({position['x']:.1f}, {position['y']:.1f}) "
            f"speed={obs['speed']:.2f} m/s "
            f"neighbors={len(neighbors)}"
        )

        for neighbor in neighbors:
            print(
                f"    -> {neighbor['vehicle_id']}: "
                f"distance={neighbor['distance']:.1f} m, "
                f"age={neighbor['age']:.2f} s"
            )


class ThreadedTCPServer( socketserver.ThreadingMixIn, socketserver.TCPServer, ):
    allow_reuse_address = True
    daemon_threads = True


if __name__ == "__main__":
    with ThreadedTCPServer((HOST, PORT), AgeNetHandler) as server:
        print(f"AgeNet bridge listening on {HOST}:{PORT}")
        server.serve_forever()
