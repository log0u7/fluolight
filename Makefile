FQBN    ?= fluo:avr:fluoeth
# Auto-detect the FLUOboard serial port (FQBN match, then VID 0x2ecf fallback).
# Override with: make <target> PORT=/dev/ttyACMx
PORT    ?= $(shell arduino-cli board list --format json 2>/dev/null | python3 -c "import json,sys;f='$(FQBN)';d=json.load(sys.stdin);[print(p['port']['address'])or sys.exit(0) for p in d.get('detected_ports',[]) for b in p.get('matching_boards',[]) if b.get('fqbn')==f];[print(p['port']['address'])or sys.exit(0) for p in d.get('detected_ports',[]) if p['port'].get('properties',{}).get('vid','').lower()=='0x2ecf']" 2>/dev/null)
BAUD    ?= 115200
SKETCH  ?= .
# Build options (overridable, match #ifndef guards in fluolight.ino)
# Example: make flash VERBOSE=1 DHCP=0 RESET_ON_FAIL=2 WATCHDOG=1
VERBOSE        ?=
DHCP           ?=
EXT_LINK_CHECK ?=
RESET_ON_FAIL  ?=
WATCHDOG       ?=

# Build extra flags from non-empty variables
_FLAGS :=
ifneq ($(VERBOSE),)
  _FLAGS += -DVERBOSE=$(VERBOSE)
endif
ifneq ($(DHCP),)
  _FLAGS += -DDHCP=$(DHCP)
endif
ifneq ($(EXT_LINK_CHECK),)
  _FLAGS += -DEXT_LINK_CHECK=$(EXT_LINK_CHECK)
endif
ifneq ($(RESET_ON_FAIL),)
  _FLAGS += -DRESET_ON_FAIL=$(RESET_ON_FAIL)
endif
ifneq ($(WATCHDOG),)
  _FLAGS += -DWATCHDOG=$(WATCHDOG)
endif

ifneq ($(_FLAGS),)
  _BUILD_PROP := --build-property compiler.cpp.extra_flags="$(_FLAGS)"
else
  _BUILD_PROP :=
endif

.DEFAULT_GOAL := help

.PHONY: help compile upload flash monitor board server clean

help:
	@echo "Usage: make <target> [VAR=value ...]"
	@echo ""
	@echo "Targets:"
	@echo "  compile   Compile the sketch"
	@echo "  upload    Upload the compiled sketch to the board"
	@echo "  flash     Compile then upload (compile + upload)"
	@echo "  monitor   Open the serial monitor ($(BAUD) baud)"
	@echo "  board     List connected boards"
	@echo "  server    Run the local test server (server/test_server.py)"
	@echo "  clean     Remove build artifacts"
	@echo ""
	@echo "Variables (overridable):"
	@echo "  FQBN=$(FQBN)"
	@echo "  PORT=$(PORT)  (auto-detected; override with PORT=/dev/ttyACMx)"
	@echo "  BAUD=$(BAUD)"
	@echo ""
	@echo "Build options (#ifndef guards - leave empty for defaults):"
	@echo "  VERBOSE=$(VERBOSE)         (0=off 1=errors 2=info+warn 3=debug)"
	@echo "  DHCP=$(DHCP)              (1=DHCP 0=static IP)"
	@echo "  EXT_LINK_CHECK=$(EXT_LINK_CHECK)  (0=auto 1=manual 2=disabled)"
	@echo "  RESET_ON_FAIL=$(RESET_ON_FAIL)   (0=off 1-3=reset on error)"
	@echo "  WATCHDOG=$(WATCHDOG)       (0=off 1=enabled)"
	@echo ""
	@echo "Examples:"
	@echo "  make flash VERBOSE=1 RESET_ON_FAIL=2 WATCHDOG=1"
	@echo "  make compile VERBOSE=0 DHCP=1"
	@echo "  make flash PORT=/dev/ttyACM1"
	@echo ""
	@echo "See BUILD_SIZES.md for the full build size matrix."

compile:
	arduino-cli config set library.enable_unsafe_install true >/dev/null 2>&1 || true
	arduino-cli lib install --git-url https://github.com/log0u7/TimedAction.git >/dev/null 2>&1 || true
	arduino-cli compile --fqbn $(FQBN) $(_BUILD_PROP) $(SKETCH)

upload:
	@test -n "$(PORT)" || { echo "No FLUOboard detected. Connect the board or pass PORT=/dev/ttyACMx"; exit 1; }
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(SKETCH)

flash: compile upload

monitor:
	@test -n "$(PORT)" || { echo "No FLUOboard detected. Connect the board or pass PORT=/dev/ttyACMx"; exit 1; }
	arduino-cli monitor -p $(PORT) -c baudrate=$(BAUD)

board:
	arduino-cli board list

server:
	python3 server/test_server.py

clean:
	arduino-cli compile --fqbn $(FQBN) --clean $(SKETCH)
