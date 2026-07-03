#!/usr/bin/env python3
"""Detect the FLUOboard serial port via arduino-cli board list --format json.

Prints the port address to stdout, or nothing if not found.
Priority: exact FQBN match, then VID 0x2ecf fallback (covers bootloader mode).

Usage (from Makefile):
  PORT := $(shell arduino-cli board list --format json 2>/dev/null | python3 detect_port.py fluo:avr:fluoeth)
"""
import json
import sys

fqbn = sys.argv[1] if len(sys.argv) > 1 else "fluo:avr:fluoeth"

try:
    data = json.load(sys.stdin)
except (json.JSONDecodeError, ValueError):
    sys.exit(0)

ports = data.get("detected_ports", [])

# 1) exact FQBN match
for p in ports:
    for b in p.get("matching_boards", []):
        if b.get("fqbn") == fqbn:
            print(p["port"]["address"])
            sys.exit(0)

# 2) fallback: Fluo VID 0x2ecf (board in bootloader mode)
for p in ports:
    if p["port"].get("properties", {}).get("vid", "").lower() == "0x2ecf":
        print(p["port"]["address"])
        sys.exit(0)
