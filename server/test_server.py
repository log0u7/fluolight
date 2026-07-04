#!/usr/bin/env python3
"""
FluoLight test server - automatically cycles through status codes 0-9.
Listens on 0.0.0.0:8080 by default, responds to GET /lights/<mac> with <N>.
The board polls every 3s (HTTP_REQ_INTERVAL); the code changes every 5s.

Status codes:
  0 - server error           (blinking red)
  1 - available + vacant     (green)
  2 - available + occupied   (orange)
  3 - booked + vacant        (pink)
  4 - booked + occupied      (red)
  5 - soon in use + vacant   (orange)
  6 - soon in use + occupied (blinking orange)
  7 - blue
  8 - white
  9 - off

Usage:
  python3 test_server.py [--port 8080] [--step 5]
  python3 test_server.py --help
"""
import argparse
import http.server
import itertools
import socketserver
import threading
import time

LABELS = {
    "0": "server error           (blinking red)",
    "1": "available + vacant     (green)",
    "2": "available + occupied   (orange)",
    "3": "booked + vacant        (pink)",
    "4": "booked + occupied      (red)",
    "5": "soon in use + vacant   (orange)",
    "6": "soon in use + occupied (blinking orange)",
    "7": "blue",
    "8": "white",
    "9": "off",
}

current = "1"


class FluoHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        body = f"<{current}>".encode()
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(body)
        print(f"  {self.client_address[0]}  {self.path}  ->  <{current}>  {LABELS[current]}")

    def log_message(self, format, *args):  # noqa: A002
        pass  # silence default access log (we have our own)


def cycle_colors(step):
    global current
    for code in itertools.cycle("0123456789"):
        current = code
        print(f"\n== code {code} : {LABELS[code]} ==")
        time.sleep(step)


def main():
    parser = argparse.ArgumentParser(
        description="FluoLight test server - cycles through status codes 0-9",
    )
    parser.add_argument(
        "--port", type=int, default=8080,
        help="HTTP port to listen on (default: 8080)",
    )
    parser.add_argument(
        "--step", type=int, default=5,
        help="Seconds each status code is displayed (default: 5)",
    )
    args = parser.parse_args()

    t = threading.Thread(target=cycle_colors, args=(args.step,), daemon=True)
    t.start()

    socketserver.TCPServer.allow_reuse_address = True
    server = socketserver.ThreadingTCPServer(("0.0.0.0", args.port), FluoHandler)

    print(f"FluoLight test server on 0.0.0.0:{args.port}")
    print(f"Cycling codes 0-9 ({args.step}s per code)")
    print("Ctrl+C to stop\n")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server.")
        server.shutdown()


if __name__ == "__main__":
    main()
