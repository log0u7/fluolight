/*
 * fluolight
 *
 * Board Manager : https://raw.githubusercontent.com/VashTheProgrammer/FLUOboard/master/package_fluo_index.json
 *  
 * This Sketch uses 20204/28672 bytes (70%) of program storage space.
 * Global variables use 669/2560 bytes (26%) of dynamic memory, leaving 1891 bytes for local variables.
 * 
 */

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
#define VERBOSE 2
#endif

/* 
 * str HTTP App Server
 * FQDN or IP address of the target server.
 * For local testing use the IP of the machine running test_server.py.
 */
#define SERVER "192.168.1.65"
// int HTTP port of the target server (test_server.py default: 8080)
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
 * int EXTERNAL LINK CHECK 
 * 0 -> AUTO    Use DNS Server provided Manualy or by DHCP  ! Memory Storage Space ~+1%
 * 1 -> MANUAL                                              ! Memory Storage Space ~+1%
 * 2 -> DISABLED
 */  
#ifndef EXT_LINK_CHECK
#define EXT_LINK_CHECK 2
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
 * 0 -> WDT DISABLE (YOU MAY WANT TO ENABLE RESET ON FAIL)
 * 1 -> WDT ENABLE 
 */
#ifndef WATCHDOG
#define WATCHDOG 0
#endif
#define WATCHDOG_TIMER WDTO_8S

/*
 * int Reset On Faill  
 * 0 -> NO (YOU SHOULD ENABLE AND SETUP WATCHDOG TIMER)
 * 1 -> RESET BOARD ON "LINK" FAIL                                              ! Memory Storage Space ~+1%
 * 2 -> RESET BOARD ON "LINK, DHCP, EXTERNAL LINK" WHEN MAX_RETRY IS REACHED    ! Memory Storage Space ~+2%
 * 3 -> RESET BOARD ON ALL ERROR FOUND WHEN MAX_RETRY IS REACHED (NEEDED?)      ! Memory Storage Space ~+2%
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
#define FADE_SPEED 5    // in ms
#define WIPE_SPEED 50   // in ms

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

// MAC ADDR INTERNAL
#define ADDR_MAC  0xFA
#define DEVICEADDRESS (0x50)
#define EE_24AA025_MAXBYTES 2048/8

/*
 *  Begin Arduino Sketch
 *  Includes Needed Libraries   
 *  
 **/ 
#include <SPI.h>                // https://www.arduino.cc/en/Reference/SPI
#include <Ethernet.h>           // https://www.arduino.cc/en/Reference/EEPROM 
#include <I2C_eeprom.h>         // https://www.arduino.cc/reference/en/libraries/i2c_eeprom/ 
#include <Adafruit_NeoPixel.h>  // https://www.arduino.cc/reference/en/libraries/adafruit-neopixel/
#if WATCHDOG == 1 || RESET_ON_FAIL > 0
#include <avr/wdt.h>            // https://www.arduino.cc/reference/en/libraries/watchdog/
#endif
#include <TimedAction.h>        // https://playground.arduino.cc/Code/TimedAction/
/*
#include <b64.h>
#include <HttpClient.h>         // https://github.com/amcewen/HttpClient
*/

/*
 *  Init Global Variables and Constants 
 */
#if MACSET == 1
static const uint8_t mac[] = MAC_ADDRESS;
#else
static uint8_t *mac;
#endif  
static uint8_t isEthLinkActive;
static uint8_t eventId;
/*  
 * Init Needed Objects
 */
// Color order: the strip is physically wired GRB (not RGB).
// NEO_RGB tells the driver to send bytes in G-R-B order, which matches the hardware.
// All Color() / setPixelColor() calls use Color(G, R, B) convention - do NOT change this.
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800); // based on led strip specs
EthernetClient client;                                             // this board use ethernet networking
//HttpClient http(client);                                         // HTTP client over Ethernet client.
#if MACSET == 0
I2C_eeprom memory(DEVICEADDRESS, EE_24AA025_MAXBYTES);             // only used to fetch mac adress from eeprom
#endif

/*
 * 
 * Arduino Setup
 * 
 **/
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize digital pin LED_BUILTIN as an output
  Serial.begin(SERIAL_BAUDS);       // Initalize Serial connection
  //delay(1000);                    // Wait board
  
  #if WATCHDOG == 1
  wdt_reset();
  wdt_enable(WATCHDOG_TIMER);       // Initalize harware reset timer
  #endif
   
  #if VERBOSE > 1
  while(!Serial);                   // Wait for serial (do not use in production)
  #endif

  strip.begin();                    // Initalize NeoPixel strip object (REQUIRED)
  strip.clear();                    // Turn OFF all pixels ASAP
  strip.show();
  strip.setBrightness(BRIGHTNESS);  // Set BRIGHTNESS
  // Display blue (Common Power ON Board
  colorWipe(strip.Color(0,0,255),2,100);
                         
  netInitSetup();                   // Initalize networking
}

/*
 * 
 * Init Protothreading Timers
 * 
 **/
TimedAction ethLinkStatus   = TimedAction( LINK_CHECK_INTERVAL,     netLinkStatus ); 
TimedAction httpRequest     = TimedAction( HTTP_REQ_INTERVAL,       httpReq ); 
TimedAction httpProcess     = TimedAction( HTTP_READ_INTERVAL,      httpRead );
TimedAction dispatchEvents  = TimedAction( EVENT_DISP_INTERVAL,     eventDispatch );       
#if DHCP == 1
TimedAction dhcpRenew       = TimedAction( DHCP_STATUS_INTERVAL,    netDhcpRenew );
#endif
#if EXT_LINK_CHECK < 2
TimedAction extLinkStatus   = TimedAction( EXT_LINK_CHECK_INTERVAL, netExtLinkStatus );
#endif

/*
 * 
 * Arduino Main Loop
 * 
 **/ 
void loop() {
  ethLinkStatus.check();
  
  if (isEthLinkActive) {
    #if EXT_LINK_CHECK < 2
    extLinkStatus.check();
    #endif
    #if DHCP == 1
    dhcpRenew.check();
    #endif     
    httpRequest.check(); 
    httpProcess.check(); 
  }
  dispatchEvents.check();
  
  #if MAIN_LOOP_INTERVAL > 0
  delay(MAIN_LOOP_INTERVAL);  // Main Clock Delay         
  #endif
  
  #if WATCHDOG == 1
  wdt_reset();                // Reset Watchdog Timer
  #endif
}
