# FluoLight

FluoLight is an Arduino-based smart lighting controller for networked status indicators. It uses addressable RGB LEDs to display different status colors based on HTTP responses from a server.

## Overview

This project is designed to create a visual indicator system that can be controlled remotely through a network interface. The device connects to a server via Ethernet, periodically polls for status updates, and displays corresponding colored lights on an LED strip. It's particularly useful for room status indicators, availability displays, or any application requiring visual status feedback.

## Hardware Requirements

- Arduino-compatible board (FLUOboard specific compatibility)
- Ethernet shield or built-in Ethernet
- Addressable LED strip (NeoPixel compatible)
- I2C EEPROM (for storing MAC address if using automatic MAC mode)

## Features

- Ethernet connectivity with both DHCP and static IP configuration options
- Remote control via HTTP requests
- Multiple status display modes with various color patterns
- Configurable refresh intervals and networking parameters
- Error recovery with optional watchdog and reset capabilities
- Detailed serial logging with configurable verbosity levels
- LED effects including color wipes, blinks, and fades

## Configuration

All configuration is done through preprocessor definitions in the `fluolight.ino` file. Below are the available configuration options:

### Serial Output Verbosity

```c
#define VERBOSE 2
```

- `0` - No output (Hardened Production Environments)
- `1` - Errors only (Production Environments) - Memory impact: ~+1%
- `2` - Errors, Warnings, Info with sync (Production Debugging) - Memory impact: ~+9%
- `3` - All output including Debug (Development Environments) - Memory impact: ~+11%

### Server Configuration

```c
#define SERVER "142.56.233.35.bc.googleusercontent.com"
#define SERVERPORT 80
```

- `SERVER` - FQDN or IP address of the HTTP server
- `SERVERPORT` - Port number for the HTTP server

### MAC Address Configuration

```c
#define MACSET 0
#define MAC_ADDRESS {0x00,0x00,0xDE,0xAD,0xBE,0xEF}
```

- `MACSET`
  - `0` - Auto (read from EEPROM) - Memory impact: ~+5%
  - `1` - Manual (use the MAC_ADDRESS defined below)
- `MAC_ADDRESS` - Six-byte MAC address (only used if MACSET=1)

### Network Configuration

```c
#define DHCP 0
#define IP_ADDRESS  {192,168,001,127}
#define IP_SUBNET   {255,255,255,000}
#define IP_GATEWAY  {192,168,001,254}
#define IP_DNS      {192,168,000,254}
```

- `DHCP`
  - `0` - Auto (use DHCP) - Memory impact: ~+13%
  - `1` - Manual (use static IP configuration below)
- `IP_ADDRESS` - Static IP address (only used if DHCP=1)
- `IP_SUBNET` - Subnet mask (only used if DHCP=1)
- `IP_GATEWAY` - Gateway IP (only used if DHCP=1)
- `IP_DNS` - DNS server IP (only used if DHCP=1)

### External Link Check

```c
#define EXT_LINK_CHECK 2
#define EXT_LINK_SERVER {9,9,9,9}
#define EXT_LINK_PORT 53
```

- `EXT_LINK_CHECK`
  - `0` - Auto (Use DNS Server provided by DHCP) - Memory impact: ~+1%
  - `1` - Manual (Use server defined in EXT_LINK_SERVER) - Memory impact: ~+1%
  - `2` - Disabled
- `EXT_LINK_SERVER` - External server IP for connectivity testing (only used if EXT_LINK_CHECK=1)
- `EXT_LINK_PORT` - Port number for external connectivity tests (typically 53 for DNS)

### Proxy Configuration

```c
#define PROXYSET 0
#define PROXY "192.168.0.253"
#define PROXYPORT 8080
```

- `PROXYSET`
  - `0` - No proxy
  - `1` - Use proxy server
- `PROXY` - Proxy server address (only used if PROXYSET=1)
- `PROXYPORT` - Proxy server port (only used if PROXYSET=1)

### Watchdog Configuration

```c
#define WATCHDOG 0
#define WATCHDOG_TIMER WDTO_8S
```

- `WATCHDOG`
  - `0` - Disabled
  - `1` - Enabled
- `WATCHDOG_TIMER` - Watchdog timeout value (e.g., WDTO_8S for 8 seconds)

### Reset On Failure Configuration

```c
#define RESET_ON_FAIL   0
#define REBOOT_TIMEOUT  4000
#define MAX_RETRY       10
```

- `RESET_ON_FAIL`
  - `0` - Disabled (you should enable WATCHDOG instead)
  - `1` - Reset on link failure - Memory impact: ~+1%
  - `2` - Reset on link, DHCP, or external link failures after MAX_RETRY attempts - Memory impact: ~+2%
  - `3` - Reset on all errors after MAX_RETRY attempts - Memory impact: ~+2%
- `REBOOT_TIMEOUT` - Time in ms to wait before rebooting (for controlled restart)
- `MAX_RETRY` - Number of failures to allow before triggering a reset

### LED Strip Configuration

```c
#define LED_PIN    6
#define LED_COUNT  7
#define BRIGHTNESS 16
#define FADE_SPEED 5
#define WIPE_SPEED 50
```

- `LED_PIN` - Arduino pin connected to the LED strip data line
- `LED_COUNT` - Number of LEDs in the strip
- `BRIGHTNESS` - LED brightness (0-255)
- `FADE_SPEED` - Speed of fading effects in ms
- `WIPE_SPEED` - Speed of wipe effects in ms

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

- `LINK_CHECK_INTERVAL` - Time between Ethernet link checks (ms)
- `EVENT_DISP_INTERVAL` - Time between event dispatching (ms)
- `DHCP_STATUS_INTERVAL` - Time between DHCP lease renewals (ms)
- `HTTP_READ_INTERVAL` - Time between processing HTTP responses (ms)
- `HTTP_REQ_INTERVAL` - Time between HTTP requests to the server (ms)
- `EXT_LINK_CHECK_INTERVAL` - Time between external connectivity checks (ms)
- `MAIN_LOOP_INTERVAL` - Delay at the end of each main loop iteration (ms)

## Status Display Codes

The device interprets numeric status codes received from the server:

- `0` - Server error (blinking fade red)
- `1` - Available + Vacant (green)
- `2` - Available + Occupied (orange)
- `3` - Booked + Vacant (pink)
- `4` - Booked + Occupied (red)
- `5` - Soon in use + Vacant (orange)
- `6` - Soon in use + Occupied (blinking orange)
- `7` - Blue
- `8` - White
- `9` - Off

## Protocol

The device makes HTTP requests to the server in the following format:

```
GET /lights/[MAC_ADDRESS] HTTP/1.1
Host: [SERVER]
Connection: close
```

The server should respond with a single digit (0-9) inside angle brackets, e.g., `<5>`, which corresponds to one of the status display codes.

## Installation

1. Configure the device by editing the define statements in `fluolight.ino`
2. Upload the sketch to your Arduino board
3. Connect the board to your network
4. The LED strip will display blue during startup
5. Once connected, the device will begin polling the server for status updates

## Troubleshooting

- If the LED displays a pulsing red color, there's a hardware or network error
- The serial monitor provides detailed information based on the VERBOSE level
- Check your network settings if the device cannot connect
- Verify server connectivity and that it's returning valid status codes

## Dependencies

The following libraries are required:
- SPI
- Ethernet
- I2C_eeprom
- Adafruit_NeoPixel
- TimedAction
- avr/wdt (if using watchdog)

## Board Compatibility

This sketch is specifically designed for the FLUOboard. To add board support, use:

```
Board Manager URL: https://raw.githubusercontent.com/VashTheProgrammer/FLUOboard/master/package_fluo_index.json
```

## Memory Usage

This sketch uses approximately:
- 20,204/28,672 bytes (70%) of program storage space
- 669/2,560 bytes (26%) of dynamic memory
