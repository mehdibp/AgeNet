#!/usr/bin/env python3
"""
Print the omnetpp.ini playground size for a SUMO network.

Usage:
    python3 tools/playground_size.py Sumo/SumoScenario/Zanjan.net.xml [margin]

Veins requires: playground = (convBoundary width/height) + 2 * margin  (default margin 25 m).
"""
import math
import sys
import xml.etree.ElementTree as ET


def main():
    net = sys.argv[1] if len(sys.argv) > 1 else "Sumo/SumoScenario/Zanjan.net.xml"
    margin = float(sys.argv[2]) if len(sys.argv) > 2 else 25.0

    loc = None
    for _, el in ET.iterparse(net, events=("start",)):
        if el.tag == "location":
            loc = el.attrib
            break
    if loc is None or "convBoundary" not in loc:
        sys.exit(f"No <location convBoundary=...> found in {net}")

    x1, y1, x2, y2 = (float(v) for v in loc["convBoundary"].split(","))
    width  = math.ceil(x2 - x1 + 2 * margin)
    height = math.ceil(y2 - y1 + 2 * margin)

    print(f"convBoundary = {x1},{y1},{x2},{y2}")
    if abs(x1) > 1e-6 or abs(y1) > 1e-6:
        print("note: boundary does not start at (0,0); Veins converts coordinates automatically.")
    print()
    print("# paste into omnetpp.ini")
    print(f"*.playgroundSizeX = {width}m")
    print(f"*.playgroundSizeY = {height}m")


if __name__ == "__main__":
    main()
