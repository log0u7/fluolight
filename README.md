# FluoLight

FluoLight is an Arduino-based smart lighting controller for networked status indicators. It uses addressable RGB LEDs to display different status colors based on HTTP responses from a server.

## Overview

This project is designed to create a visual indicator system that can be controlled remotely through a network interface. The device connects to a server via Ethernet, periodically polls for status updates, and displays corresponding colored lights on an LED strip. It is particularly useful for room status indicators, availability displays, or any application requiring visual status feedback.

## Hardware Requirements

- FLUOboard (Fluo Technology, ATmega32U4 with built-in Ethernet)
- Addressable LED strip (NeoPixel compatible, WS2812)
- I2C EEPROM (for storing MAC address when using automatic MAC mode)
- USB cable with data lines (not a charge-only cable)

## Features

- Ethernet connectivity with both DHCP and static IP configuration options
- Remote control via HTTP requests
- Multiple status display modes with various color patterns
- Configurable refresh intervals and networking parameters
- Error recovery with optional watchdog and reset capabilities
- Detailed serial logging with configurable verbosity levels
- LED effects including color wipes, blinks, and fades

## Build

### Requirements

- [arduino-cli](https://arduino.github.io/arduino-cli/) installed and in `PATH`
- FLUOboard core (`fluo:avr`) installed (see Board Setup below)
- Required libraries installed (see Dependencies below)
- `python3` for the test server

### Board Setup

Add the FLUOboard package URL to arduino-cli once:

```bash
arduino-cli config add board_manager.additional_urls \
  https://raw.githubusercontent.com/VashTheProgrammer/FLUOboard/master/package_fluo_index.json
arduino-cli core update-index
arduino-cli core install fluo:avr
```

Board FQBN: `fluo:avr:fluoeth`

### Common commands (via Makefile)

```bash
make          # show available targets
make compile  # compile only
make flash    # compile + upload (port auto-detected)
make monitor  # open serial monitor at 115200 baud
make server   # run the local test server
make board    # list connected boards
```

The serial port is auto-detected from `arduino-cli board list` (FQBN
match, then VID fallback). Override if needed:

```bash
make flash PORT=/dev/ttyACM1
```

### Manual arduino-cli commands

```bash
arduino-cli compile --fqbn fluo:avr:fluoeth .
arduino-cli upload  -p /dev/ttyACM0 --fqbn fluo:avr:fluoeth .
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

## Configuration

All configuration is done through preprocessor definitions in `fluolight.ino`.

### Serial Output Verbosity

```c
#define VERBOSE 2
```

- `0` - No output (hardened production)
- `1` - Errors only (production) - flash impact: ~+1%
- `2` - Errors, warnings, info with serial sync (debug) - flash impact: ~+9%
- `3` - All output including debug - flash impact: ~+11%

Note: when `VERBOSE > 1`, the board waits for a serial connection before starting (`while(!Serial)`). Set `VERBOSE` to `0` or `1` for standalone deployment without a PC.

### Server Configuration

```c
#define SERVER "your-server-address"
#define SERVERPORT 8080
```

- `SERVER` - FQDN or IP address of the HTTP server
- `SERVERPORT` - Port number of the HTTP server (default: `8080`, matches `server/test_server.py`)

The board makes `GET /lights/<mac>` requests to this server every `HTTP_REQ_INTERVAL` ms and expects a response body containing `<N>` where `N` is a digit 0-9 (see Status Display Codes).

### MAC Address Configuration

```c
#define MACSET 0
#define MAC_ADDRESS {0x00,0x00,0xDE,0xAD,0xBE,0xEF}
```

- `MACSET`
  - `0` - Auto (read from I2C EEPROM) - flash impact: ~+5%
  - `1` - Manual (use `MAC_ADDRESS` below)
- `MAC_ADDRESS` - Six-byte MAC address (only used if `MACSET=1`)

### Network Configuration

```c
#define DHCP 1
#define IP_ADDRESS  {192,168,001,127}
#define IP_SUBNET   {255,255,255,000}
#define IP_GATEWAY  {192,168,001,254}
#define IP_DNS      {192,168,000,254}
```

- `DHCP`
  - `0` - Manual (use static IP configuration below)
  - `1` - Auto (use DHCP) - flash impact: ~+13%

Note: with `DHCP=0`, `Ethernet.begin()` blocks for up to ~60s at boot if no network cable is connected.

### External Link Check

```c
#define EXT_LINK_CHECK 2
#define EXT_LINK_SERVER {9,9,9,9}
#define EXT_LINK_PORT 53
```

- `EXT_LINK_CHECK`
  - `0` - Auto (use DNS server from DHCP) - flash impact: ~+1%
  - `1` - Manual (use `EXT_LINK_SERVER`) - flash impact: ~+1%
  - `2` - Disabled

### Watchdog Configuration

```c
#define WATCHDOG 0
#define WATCHDOG_TIMER WDTO_8S
```

- `WATCHDOG`
  - `0` - Disabled
  - `1` - Enabled (hardware watchdog via `avr/wdt.h`)

### Reset On Failure

```c
#define RESET_ON_FAIL   0
#define REBOOT_TIMEOUT  4000
#define MAX_RETRY       10
```

- `RESET_ON_FAIL`
  - `0` - Disabled
  - `1` - Reset on link failure - flash impact: ~+1%
  - `2` - Reset on link, DHCP, or external link failures after `MAX_RETRY` attempts - flash impact: ~+2%
  - `3` - Reset on all errors after `MAX_RETRY` attempts - flash impact: ~+2%
- `REBOOT_TIMEOUT` - Time in ms before a long reset triggers
- `MAX_RETRY` - Number of consecutive failures before reset

### LED Strip Configuration

```c
#define LED_PIN    6
#define LED_COUNT  7
#define BRIGHTNESS 16
#define FADE_SPEED 5
#define WIPE_SPEED 50
```

### Color order (GRB)

The LED strip is physically wired **GRB** (green-red-blue), not RGB. The driver is
initialized with `NEO_RGB` so that bytes are sent in G-R-B order to match the hardware.

As a result, every `Color()` and `setPixelColor()` call in the sketch passes arguments
as `(G, R, B)` rather than the more common `(R, G, B)`. For example, pure red is
`Color(0, 255, 0)` and pure green is `Color(255, 0, 0)`.

**Do not "correct" these values or change `NEO_RGB` to `NEO_GRB`.** The colors render
correctly on the hardware as-is. Changing either the flag or the argument order would
invert red and green on the strip.

### Timing Intervals

```c
#define LINK_CHECK_INTERVAL       500
#define EVENT_DISP_INTERVAL       50
#define DHCP_STATUS_INTERVAL      10*60000
#define HTTP_READ_INTERVAL        600
#define HTTP_REQ_INTERVAL         3000
#define EXT_LINK_CHECK_INTERVAL   25000
#define MAIN_LOOP_INTERVAL        0
```

## Status Display Codes

### App codes (server response)

The device interprets numeric codes received from the server (inside `<N>`).
All app animations are non-blocking: the HTTP polling loop keeps running during
any LED effect. An animation replays only when the server code changes.

| Code | Meaning | LED effect |
|------|---------|------------|
| `0` | Server error | Continuous fade red |
| `1` | Available + vacant | Wipe green |
| `2` | Available + occupied | Wipe orange |
| `3` | Booked + vacant | Wipe pink |
| `4` | Booked + occupied | Wipe red |
| `5` | Soon in use + vacant | Wipe orange |
| `6` | Soon in use + occupied | Continuous toggle green/orange |
| `7` | (user-defined) | Wipe blue |
| `8` | (user-defined) | Wipe white |
| `9` | Off | Wipe black |

### System status events (board-level)

The board emits its own LED signals independently of the server to indicate
network and hardware state. These events are also non-blocking and use the
same animation state machine.

| Event | Semantic | LED effect | Trigger |
|-------|---------|------------|---------|
| 1 | Board error | Fade red | Any network error when `RESET_ON_FAIL > 0` |
| 2 | Ethernet init OK | Wipe green (1 pixel) | Boot, after DHCP/static IP assigned |
| 3 | Ext TCP check OK | Flash white | `EXT_LINK_CHECK < 2`, every 25s, target reachable |
| 4 | Ext TCP check KO | Flash orange | `EXT_LINK_CHECK < 2`, every 25s, target unreachable |
| 5 | DHCP renew/rebind OK | Fade green | `DHCP=1`, every 10min, lease renewed |
| 6 | DHCP renew/rebind KO | Fade red | `DHCP=1`, every 10min, lease failed |

Board reset: the strip briefly fills blue (~300ms) just before the watchdog
triggers a reboot. This happens in `reset()` directly, independently of the
event system.

## Protocol

The device makes HTTP requests in the following format:

```
GET /lights/<mac_address> HTTP/1.1
Host: <SERVER>
Connection: close
```

The server must respond with a body containing `<N>` where `N` is a digit 0-9, for example `<1>`.

## Test Server

A local test server is provided to validate the LED display without a production server.

```bash
make server
# or directly:
python3 server/test_server.py
```

The server listens on `0.0.0.0:8080` and cycles automatically through all 10 status codes (one per request received from the board). Each `GET /lights/<mac>` request advances the cycle by one step.

Make sure port 8080 is open in your firewall:

```bash
sudo ufw allow from 192.168.1.0/24 to any port 8080 proto tcp
```

## Pipeline Status Server

`server/pipeline_server.py` is a production-ready server that polls GitHub Actions
and GitLab CI pipelines and exposes their state to the board via the same
`GET /lights/<mac>` protocol. Each board (identified by its MAC address) is
mapped to one CI/CD project. `server/test_server.py` remains available for
offline testing.

### Requirements

- Python 3.6+ (stdlib only, no extra packages)
- A GitHub personal access token (for GitHub targets)
- A GitLab personal access token (for GitLab targets)

### Configuration

Copy `server/targets.example.json` to `targets.json` (gitignored) and fill in your
targets:

```json
{
  "port": 8080,
  "poll_interval": 30,
  "targets": {
    "5410ecee3ea0": {
      "provider": "github",
      "repo": "owner/repository",
      "branch": "main"
    },
    "aabbccddeeff": {
      "provider": "gitlab",
      "host": "gitlab.com",
      "project": "group/repository",
      "branch": "main"
    }
  }
}
```

The MAC address key must match the board's MAC in lowercase without
separators (printed at boot: `INF:ETH:INIT: 5410ecee3ea0`).

For self-hosted GitLab instances, set `host` to your instance hostname.

### Authentication

Set tokens as environment variables - never put them in `targets.json`:

```bash
export GITHUB_TOKEN=ghp_...
export GITLAB_TOKEN=glpat-...
```

For multiple self-hosted GitLab instances, use per-host variables
(dots replaced by underscores):

```bash
export GITLAB_TOKEN_gitlab_example_com=glpat-...
```

### Running

```bash
python3 server/pipeline_server.py                    # uses targets.json on port 8080
python3 server/pipeline_server.py --config /path/to/cfg.json
python3 server/pipeline_server.py --help             # list all options
```

Open the firewall if needed:

```bash
sudo ufw allow from 192.168.1.0/24 to any port 8080 proto tcp
```

### Status code mapping

| Code | Color | GitHub Actions | GitLab CI |
|------|-------|---------------|-----------|
| `0` | Blinking red | API error / network failure / MAC not in config | same |
| `1` | Green | completed / success | success |
| `4` | Red | completed / failure or timed_out | failed |
| `6` | Blinking orange | in_progress / queued / pending | running / pending / created |
| `8` | White | canceled / skipped / neutral | canceled / skipped / manual |
| `9` | Off | no run found | no pipeline found |

On transient API errors the last known state is preserved so the board
does not flash red on a momentary network blip.

## Installation

1. Install arduino-cli and the FLUOboard core (see Board Setup above).
2. Install the required libraries (see Dependencies below).
3. Configure `fluolight.ino` (server address, port, MAC mode, network settings).
4. Compile and upload: `make flash`
5. Connect the board to your network (Ethernet cable required at boot when using DHCP).
6. Open the serial monitor to verify startup: `make monitor`
7. The LED strip displays blue during startup, then follows server responses.

## Troubleshooting

- **Board not detected at all** (`/dev/ttyACM0` missing): the USB cable is charge-only (no data lines). Replace with a full USB cable.
- **No serial output after connect**: with `VERBOSE > 1` the board waits for a serial connection before running. Open the monitor then press the reset button on the board.
- **LED stuck on orange/red at boot, no HTTP activity**: board is waiting for DHCP with no network cable connected. Connect the Ethernet cable before powering on.
- **`ERR:HTTP:SND:KO`**: server unreachable. Check `SERVER`/`SERVERPORT` in `fluolight.ino`, verify the server is running, and check the firewall on the server machine (port 8080).
- **LED displays pulsing red**: board error event (system event 1) or server error code `<0>`. Check serial output for `ERR:` lines.
- **Upload fails with "port not found"**: the 32U4 bootloader may have timed out. Double-press the reset button to force bootloader mode, then retry `make flash`.
- **`brltty` installed on Ubuntu 24.04**: this can interfere with CH340-based boards but does not affect the FLUOboard (ATmega32U4, `cdc_acm` driver).

## Dependencies

- SPI (bundled with Arduino AVR core)
- Ethernet (bundled with Arduino AVR core)
- I2C_EEPROM
- Adafruit NeoPixel
- TimedAction
- avr/wdt (bundled, used when `WATCHDOG=1` or `RESET_ON_FAIL > 0`)

## Board Compatibility

This sketch targets the FLUOboard (Fluo Technology, ATmega32U4 with built-in Ethernet).

Board Manager URL:
```
https://raw.githubusercontent.com/VashTheProgrammer/FLUOboard/master/package_fluo_index.json
```

## Memory Usage

With the default configuration (`VERBOSE=2`, `DHCP=0`, `EXT_LINK_CHECK=2`, `RESET_ON_FAIL=0`, `WATCHDOG=0`):

- Program storage: ~25,880 / 28,672 bytes (90%)
- Dynamic memory: ~912 / 2,560 bytes (35%)

Flash usage is sensitive to verbosity and network options (see flash impact notes in each section above). With `VERBOSE=1` approximately 9% of flash is recovered.

### Build size matrix

The five main options (`VERBOSE`, `DHCP`, `EXT_LINK_CHECK`, `RESET_ON_FAIL`, `WATCHDOG`) are guarded by `#ifndef` in `fluolight.ino` and can be overridden at compile time:

```bash
arduino-cli compile --fqbn fluo:avr:fluoeth \
  --build-property compiler.cpp.extra_flags="-DVERBOSE=1 -DDHCP=0 -DRESET_ON_FAIL=2 -DWATCHDOG=1" .
```

See [`BUILD_SIZES.md`](BUILD_SIZES.md) for the exhaustive build size matrix (192 combinations) and named profiles (prod minimal, prod robuste, dev).
