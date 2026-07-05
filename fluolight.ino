/*
 * fluolight
 *
 * Board Manager : https://raw.githubusercontent.com/VashTheProgrammer/FLUOboard/master/package_fluo_index.json
 *
 */

#include "config.h"

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
#if EXT_LINK_CHECK != 0
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
    #if EXT_LINK_CHECK != 0
    extLinkStatus.check();
    #endif
    #if DHCP == 1
    dhcpRenew.check();
    #endif     
    httpRequest.check(); 
    httpProcess.check(); 
  }
  dispatchEvents.check();
  animTick();
  
  #if MAIN_LOOP_INTERVAL > 0
  delay(MAIN_LOOP_INTERVAL);  // Main Clock Delay         
  #endif
  
  #if WATCHDOG == 1
  wdt_reset();                // Reset Watchdog Timer
  #endif
}
