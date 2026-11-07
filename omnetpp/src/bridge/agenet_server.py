import json, socketserver

HOST, PORT = "0.0.0.0", 5555


class AgeNetHandler(socketserver.StreamRequestHandler):
    def handle(self):
        print(f"[CONNECT] {self.client_address}")

        try:
            while True:
                line = self.rfile.readline()
                if not line: break

                try:
                    observation: dict = json.loads(line)
                    if observation.get("type") != "observe": raise ValueError("Expected an observe message")

                    vehicles: dict = observation["vehicles"]        # {vid: {"position","speed","road_id","neighbors"}}
                    self.print_observation(observation)

                    response = {
                        "type": "action", "step": observation["step"], "sim_time": observation["sim_time"],
                        "actions": {vid: {"txPower_mW": 17.5} for vid in vehicles},   # dummy
                    }
                except (ValueError, KeyError, TypeError) as exc:
                    print(f"[INVALID MESSAGE] {exc}")
                    response = {"type": "error", "message": "invalid_message"}

                self.wfile.write((json.dumps(response, separators=(",", ":")) + "\n").encode())
                self.wfile.flush()
        finally:
            print(f"[DISCONNECT] {self.client_address}")
            

    @staticmethod
    def print_observation(observation: dict):
        sim_time = observation["sim_time"]

        vehicles: dict = observation["vehicles"]
        vehicle_ids = vehicles.keys()

        print(f"[OBS] step={observation['step']} t={sim_time:.1f}s vehicles={len(vehicle_ids)}")
        for vid in vehicle_ids:
            vehicle_id  = vid
            position  = vehicles[vid]["position"]
            speed     = vehicles[vid]["speed"]
            road_id   = vehicles[vid]["road_id"]
            neighbors = vehicles[vid]["neighbors"]

            print(
                f"[OBS] t={sim_time:.1f}s "
                f"vehicle={vehicle_id} "
                f"position=({position}) "
                f"speed={speed:.2f} m/s "
                f"road_id={road_id} \t"
                f"neighbors={len(neighbors)}"
            )

            for neighbor in neighbors:
                print(
                    f"      -> {neighbor['vehicle_id']}: "
                    f"position={neighbor['position']} "
                    f"distance={neighbor['distance']:.1f} m, "
                    f"age={neighbor['age']:.2f} s"
                )


class Server(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True
    daemon_threads = True


if __name__ == "__main__":
    with Server((HOST, PORT), AgeNetHandler) as server:
        print(f"AgeNet bridge listening on {HOST}:{PORT}")
        server.serve_forever()
