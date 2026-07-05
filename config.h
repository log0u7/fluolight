#ifndef CONFIG_H
#define CONFIG_H

/*
 * Preprocessor Definitions
 * Provide quick board configuration 
 * 
 **/
  
/* 
 * int Serial Output Verbosity
 * 0 No Output                          (Hardened Production Environments)
 * 1 Errors w/out sync                  (Production Environments)             ! Memory Storage Space ~+1%
 * 2 Errors, Warn, Info w sync          (Production Environments Debuging)    ! Memory Storage Space ~+9%
 * 3 Errors, Warn, Info & Debug w/ sync (Devel Environments w/ Serial Only )  ! Memory Storage Space ~+11% 
 */
#ifndef VERBOSE
#define VERBOSE 1
#endif

/* 
 * str HTTP App Server
 * FQDN or IP address of the target server.
 * For local testing use the IP of the machine running server/test_server.py.
 */
#define SERVER "192.168.1.65"
// int HTTP port of the target server (server/test_server.py default: 8080)
#define SERVERPORT 8080

/* 
 * bool MAC ADDR SETTING 
 * 0 -> AUTO(EEPROM MACADDR)        ! Memory Storage Space ~+5%
 * 1 -> MANUAL - MUST SET MAC ADDR  
 */
#define MACSET 0

/* 
 * ETHER BOARD SETUP  
 * array MAC_ADDRESS
 */
#define MAC_ADDRESS {0x00,0x00,0xDE,0xAD,0xBE,0xEF}

/* 
 * bool NETWORK SETTING 
 * 0 -> MANUAL (DHCP OFF, static IP) 
 * 1 -> AUTO   (DHCP ON)             ! Memory Storage Space ~+13%
 */
#ifndef DHCP
#define DHCP 1
#endif

/* 
 * IP BOARD SETUP
 * array IP_ADDRESS 
 * array IP_SUBNET
 * array IP_GATEWAY
 * array IP_DNS
 */
#define IP_ADDRESS  {192,168,001,127}
#define IP_SUBNET   {255,255,255,000}
#define IP_GATEWAY  {192,168,001,254}
#define IP_DNS      {192,168,000,254}

/*
 * int External link check
 * 0 -> DISABLED
 * 1 -> AUTO    Use DNS Server provided by DHCP              ! Memory Storage Space ~+1%
 * 2 -> MANUAL  Use EXT_LINK_SERVER (default 9.9.9.9:53)     ! Memory Storage Space ~+1%
 */
#ifndef EXT_LINK_CHECK
#define EXT_LINK_CHECK 0
#endif

/* 
 * array TCP Check Server
 * IP ADDRESS 
 */
#define EXT_LINK_SERVER {9,9,9,9}
// int SERVER_PORT
#define EXT_LINK_PORT 53

/* 
 * bool PROXY SETTING
 * 0 -> NO PROXY / 
 * 1 -> PROXY ENABLE - MUST SET THE PROXY PORT 
 */
#define PROXYSET 0

/* 
 * str HTTP Proxy Server
 * FQDN |IP ADDRESS 
 */
#define PROXY "192.168.0.253"
// int SERVER_PORT
#define PROXYPORT 8080

/* 
 * bool Hardware Watchdog Timer
 * 0 -> WDT DISABLED (enable RESET_ON_FAIL instead)
 * 1 -> WDT ENABLED (auto-reboot if CPU freezes, 8s timeout)
 *
 * At least one of WATCHDOG or RESET_ON_FAIL must be enabled
 * for unattended production use. Enabling both is safe.
 * WATCHDOG covers CPU freeze; RESET_ON_FAIL covers network
 * error loops.
 */
#ifndef WATCHDOG
#define WATCHDOG 0
#endif
#define WATCHDOG_TIMER WDTO_8S

/*
 * int Reset On Failure
 * 0 -> DISABLED (enable WATCHDOG instead)
 * 1 -> Reset on link failure                     flash ~+1%
 * 2 -> Reset on link, DHCP, external link fail   flash ~+2%
 * 3 -> Reset on all errors                       flash ~+2%
 */
#ifndef RESET_ON_FAIL
#define RESET_ON_FAIL 0
#endif
#define REBOOT_TIMEOUT  4000
#define MAX_RETRY       10  // before restart

// NEO PIXEL CONFIGURATION
#define LED_PIN    6    // Led pin of the strip
#define LED_COUNT  7    // number of led on strip
#define BRIGHTNESS 16   // 0 (min) to 255 (max)
#define FADE_SPEED 8    // in ms per step (510 steps = ~4s per fade cycle)
#define WIPE_SPEED 70   // in ms per pixel (7 pixels = ~0.5s full wipe)

// SERIAL BITRATE
#define SERIAL_BAUDS 115200 //9600

// PROTOTHREADING INTERVALS (MS)
#define LINK_CHECK_INTERVAL       500       // 500ms between ethernet link check
#define EVENT_DISP_INTERVAL       50        // events dispatching every 50ms
#define DHCP_STATUS_INTERVAL      10*60000  // 15 minutes between dhcp renew
#define HTTP_READ_INTERVAL        600       // process any http data received from server each 600ms
#define HTTP_REQ_INTERVAL         3000      // wait 3 secondes between http request sent to server
#define EXT_LINK_CHECK_INTERVAL   25000     // 25 secondes between each external tcp checks
#define MAIN_LOOP_INTERVAL        0         // Mais loop delay : 0 - 100ms

// HTTP consecutive failure threshold before logging KO
#define HTTP_FAIL_THRESHOLD       3

// MAC ADDR INTERNAL
#define ADDR_MAC  0xFA
#define DEVICEADDRESS (0x50)
#define EE_24AA025_MAXBYTES 2048/8

#endif
