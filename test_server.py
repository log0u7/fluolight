#!/usr/bin/env python3
"""
Serveur de test fluolight - cycle automatique des codes couleur 0-9
Ecoute sur 0.0.0.0:8080, repond a GET /lights/<mac> avec <N>
La carte interroge toutes les 3s (HTTP_REQ_INTERVAL), le code change toutes les 5s.

Codes couleur :
  0 - erreur serveur (rouge clignotant)
  1 - disponible + libre (vert)
  2 - disponible + occupe (orange)
  3 - reserve + libre (rose)
  4 - reserve + occupe (rouge)
  5 - bientot utilise + libre (orange)
  6 - bientot utilise + occupe (orange clignotant)
  7 - bleu
  8 - blanc
  9 - eteint
"""
import http.server
import socketserver
import threading
import time
import itertools

PORT = 8080
STEP_SECONDS = 5   # duree d'affichage par couleur (en secondes)

LABELS = {
    "0": "erreur serveur       (rouge clignotant)",
    "1": "disponible + libre   (vert)",
    "2": "disponible + occupe  (orange)",
    "3": "reserve + libre      (rose)",
    "4": "reserve + occupe     (rouge)",
    "5": "bientot utilise      (orange)",
    "6": "bientot + occupe     (orange clignotant)",
    "7": "bleu",
    "8": "blanc",
    "9": "eteint",
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

print(f"Serveur fluolight sur 0.0.0.0:{PORT}")
print(f"Cycle automatique des codes 0-9 ({STEP_SECONDS}s par couleur)")
print("Ctrl+C pour arreter\n")

try:
    server.serve_forever()
except KeyboardInterrupt:
    print("\nArret du serveur.")
    server.shutdown()
