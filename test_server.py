#!/usr/bin/env python3
"""
FluoLight test server - automatically cycles through status codes 0-9.
Listens on 0.0.0.0:8080, responds to GET /lights/<mac> with <N>.
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
"""
import http.server
import socketserver
import threading
import time
import itertools

PORT = 8080
STEP_SECONDS = 5   # duree d'affichage par couleur (en secondes)

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
        pass   # silence le log par defaut (on a le notre)


def cycle_colors():
    global current
    for code in itertools.cycle("0123456789"):
        current = code
        print(f"\n== code {code} : {LABELS[code]} ==")
        time.sleep(STEP_SECONDS)


socketserver.TCPServer.allow_reuse_address = True
server = socketserver.ThreadingTCPServer(("0.0.0.0", PORT), FluoHandler)

t = threading.Thread(target=cycle_colors, daemon=True)
t.start()

print(f"FluoLight test server on 0.0.0.0:{PORT}")
print(f"Cycling codes 0-9 ({STEP_SECONDS}s per code)")
print("Ctrl+C to stop\n")

try:
    server.serve_forever()
except KeyboardInterrupt:
    print("\nStopping server.")
    server.shutdown()
