#!/usr/bin/env python3
"""
FluoLight pipeline status server - multi-target CI/CD state to LED code.

Polls GitHub Actions and GitLab CI pipelines at a configurable interval,
maps pipeline state to a FluoLight status code (0-9), and serves it to
the board via the standard GET /lights/<mac> -> <N> protocol.

Each board (identified by its MAC address) is mapped to one CI/CD target.
Configuration is read from a JSON file (default: targets.json, gitignored).
Tokens are read exclusively from environment variables, never from config.

Environment variables:
  GITHUB_TOKEN   Personal access token for GitHub API (required for GitHub targets)
  GITLAB_TOKEN   Personal access token for GitLab API (required for GitLab targets)
                 For self-hosted instances use GITLAB_TOKEN_<HOST> where <HOST>
                 is the hostname with dots replaced by underscores, e.g.
                 GITLAB_TOKEN_gitlab_example_com

Status code mapping:
  0  error / unknown / MAC not in config    (blinking red)
  1  pipeline success                       (green)
  4  pipeline failed / timed out            (red)
  6  pipeline running / pending / queued    (blinking orange)
  8  pipeline canceled / skipped / manual   (white)
  9  no pipeline found                      (off)

Usage:
  python3 pipeline_server.py [--config targets.json] [--port 8080]
                             [--host 0.0.0.0] [--poll-interval 30]
"""

import argparse
import json
import logging
import os
import socketserver
import sys
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
from http.server import BaseHTTPRequestHandler

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s  %(levelname)-8s  %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("pipeline_server")

# ---------------------------------------------------------------------------
# Status code mapping
# ---------------------------------------------------------------------------

CODE_ERROR   = "0"   # error / unknown / MAC not in config
CODE_SUCCESS = "1"   # success
CODE_FAILED  = "4"   # failed / timed out
CODE_RUNNING = "6"   # running / pending / queued / created
CODE_CANCEL  = "8"   # canceled / skipped / manual
CODE_NONE    = "9"   # no pipeline found


def _github_state(status, conclusion):
    """Map GitHub Actions run status + conclusion to a FluoLight code."""
    if status in ("queued", "in_progress", "waiting", "requested", "pending"):
        return CODE_RUNNING
    if status == "completed":
        if conclusion in ("success",):
            return CODE_SUCCESS
        if conclusion in ("failure", "timed_out", "startup_failure"):
            return CODE_FAILED
        if conclusion in ("cancelled",):
            return CODE_CANCEL
        if conclusion in ("skipped", "neutral", "action_required"):
            return CODE_CANCEL
    return CODE_ERROR


def _gitlab_state(status):
    """Map GitLab pipeline status to a FluoLight code."""
    if status in ("success",):
        return CODE_SUCCESS
    if status in ("failed",):
        return CODE_FAILED
    if status in ("running", "pending", "created", "waiting_for_resource", "preparing"):
        return CODE_RUNNING
    if status in ("canceled", "skipped", "manual", "scheduled"):
        return CODE_CANCEL
    return CODE_ERROR

# ---------------------------------------------------------------------------
# API polling
# ---------------------------------------------------------------------------

GITHUB_API = "https://api.github.com"
_REQUEST_TIMEOUT = 10  # seconds


def _get(url, headers):
    """Perform a GET request, return parsed JSON or raise on error."""
    req = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(req, timeout=_REQUEST_TIMEOUT) as resp:
        if resp.status != 200:
            raise ValueError(f"HTTP {resp.status} for {url}")
        return json.loads(resp.read().decode())


def _github_token():
    tok = os.environ.get("GITHUB_TOKEN", "")
    if not tok:
        raise EnvironmentError("GITHUB_TOKEN is not set")
    return tok


def _gitlab_token(host):
    key = "GITLAB_TOKEN_" + host.replace(".", "_")
    tok = os.environ.get(key) or os.environ.get("GITLAB_TOKEN", "")
    if not tok:
        raise EnvironmentError(
            f"No GitLab token found. Set GITLAB_TOKEN or {key}"
        )
    return tok


def poll_github(target):
    """Poll one GitHub Actions target, return a status code string."""
    repo   = target["repo"]
    branch = target.get("branch", "main")
    token  = _github_token()
    url = (
        f"{GITHUB_API}/repos/{repo}/actions/runs"
        f"?branch={urllib.parse.quote(branch, safe='')}&per_page=1"
    )
    headers = {
        "Authorization": f"Bearer {token}",
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
    }
    data = _get(url, headers)
    runs = data.get("workflow_runs", [])
    if not runs:
        return CODE_NONE
    run = runs[0]
    return _github_state(run.get("status", ""), run.get("conclusion") or "")


def poll_gitlab(target):
    """Poll one GitLab CI target, return a status code string."""
    host    = target.get("host", "gitlab.com")
    project = target["project"]
    branch  = target.get("branch", "main")
    token   = _gitlab_token(host)
    encoded = urllib.parse.quote(project, safe="")
    url = (
        f"https://{host}/api/v4/projects/{encoded}/pipelines"
        f"?ref={urllib.parse.quote(branch, safe='')}&per_page=1"
    )
    headers = {"PRIVATE-TOKEN": token}
    data = _get(url, headers)
    if not data:
        return CODE_NONE
    return _gitlab_state(data[0].get("status", ""))


_POLLERS = {
    "github": poll_github,
    "gitlab": poll_gitlab,
}

# ---------------------------------------------------------------------------
# Shared cache
# ---------------------------------------------------------------------------

class StatusCache:
    """Thread-safe map: mac -> (code, timestamp). Last known value is kept
    on transient errors so a blip does not reset a green board to red."""

    def __init__(self):
        self._lock = threading.Lock()
        self._data = {}   # mac -> (code, timestamp)

    def update(self, mac, code):
        with self._lock:
            self._data[mac] = (code, time.time())

    def get(self, mac):
        """Return current code, or CODE_ERROR if MAC is not in config."""
        with self._lock:
            entry = self._data.get(mac)
        if entry is None:
            return CODE_ERROR
        return entry[0]

    def set_initial(self, mac):
        """Mark a MAC as known-but-not-yet-polled (prevents instant CODE_ERROR)."""
        with self._lock:
            if mac not in self._data:
                self._data[mac] = (CODE_NONE, 0.0)


# ---------------------------------------------------------------------------
# Polling thread
# ---------------------------------------------------------------------------

def _poll_loop(targets_cfg, cache, interval):
    """Poll all targets in a tight loop with the configured interval."""
    while True:
        for mac, target in targets_cfg.items():
            provider = target.get("provider", "").lower()
            poller   = _POLLERS.get(provider)
            if poller is None:
                log.error("Unknown provider %r for MAC %s", provider, mac)
                continue
            try:
                code = poller(target)
                cache.update(mac, code)
                log.info(
                    "%-17s  %-8s  %s/%s  -> code %s",
                    mac,
                    provider,
                    target.get("repo") or target.get("project", "?"),
                    target.get("branch", "main"),
                    code,
                )
            except urllib.error.HTTPError as exc:
                log.error(
                    "HTTP %s for MAC %s (%s): %s",
                    exc.code, mac, provider, exc.reason,
                )
            except urllib.error.URLError as exc:
                log.error("Network error for MAC %s: %s", mac, exc.reason)
            except Exception as exc:  # noqa: BLE001
                # Covers config errors (missing token etc.) and anything else.
                # Do NOT overwrite cache on error - keep last known value.
                log.error("Error polling MAC %s (%s): %s", mac, provider, exc)

        time.sleep(interval)


# ---------------------------------------------------------------------------
# HTTP server
# ---------------------------------------------------------------------------

def _make_handler(cache):
    class FluoHandler(BaseHTTPRequestHandler):
        def do_GET(self):
            # Expect /lights/<mac>
            parts = self.path.strip("/").split("/")
            if len(parts) == 2 and parts[0] == "lights":
                mac  = parts[1].lower()
                code = cache.get(mac)
                body = f"<{code}>".encode()
                self.send_response(200)
                self.send_header("Content-Type", "text/plain")
                self.send_header("Content-Length", str(len(body)))
                self.send_header("Connection", "close")
                self.end_headers()
                self.wfile.write(body)
                log.debug("%-17s  -> <%s>", mac, code)
            else:
                self.send_response(404)
                self.end_headers()

        def log_message(self, format, *args):  # noqa: A002
            pass  # silence default access log (we have our own)

    return FluoHandler


# ---------------------------------------------------------------------------
# Config loading
# ---------------------------------------------------------------------------

def load_config(path):
    try:
        with open(path) as fh:
            return json.load(fh)
    except FileNotFoundError:
        log.error("Config file not found: %s", path)
        sys.exit(1)
    except json.JSONDecodeError as exc:
        log.error("Invalid JSON in %s: %s", path, exc)
        sys.exit(1)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="FluoLight pipeline status server",
    )
    parser.add_argument(
        "--config", default="targets.json",
        help="Path to targets config file (default: targets.json)",
    )
    parser.add_argument(
        "--port", type=int, default=None,
        help="HTTP port to listen on (overrides config, default: 8080)",
    )
    parser.add_argument(
        "--host", default="0.0.0.0",
        help="Address to bind (default: 0.0.0.0)",
    )
    parser.add_argument(
        "--poll-interval", type=int, default=None,
        help="Seconds between API polls (overrides config, default: 30)",
    )
    args = parser.parse_args()

    cfg      = load_config(args.config)
    port     = args.port          or cfg.get("port", 8080)
    interval = args.poll_interval or cfg.get("poll_interval", 30)
    targets  = cfg.get("targets", {})

    if not targets:
        log.error("No targets defined in %s", args.config)
        sys.exit(1)

    # Normalise MAC keys to lowercase
    targets = {k.lower(): v for k, v in targets.items()}

    cache = StatusCache()
    for mac in targets:
        cache.set_initial(mac)

    # Start polling thread
    t = threading.Thread(
        target=_poll_loop,
        args=(targets, cache, interval),
        daemon=True,
    )
    t.start()

    # Start HTTP server
    socketserver.TCPServer.allow_reuse_address = True
    server = socketserver.ThreadingTCPServer(
        (args.host, port), _make_handler(cache)
    )

    log.info("FluoLight pipeline server on %s:%d", args.host, port)
    log.info("Polling %d target(s) every %ds", len(targets), interval)
    for mac, tgt in targets.items():
        log.info(
            "  %s  ->  %s  %s/%s",
            mac,
            tgt.get("provider", "?"),
            tgt.get("repo") or tgt.get("project", "?"),
            tgt.get("branch", "main"),
        )

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        log.info("Stopping server.")
        server.shutdown()


if __name__ == "__main__":
    main()
