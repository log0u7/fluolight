/*
 * 
 *  LED Display Event Function
 *
 **/

void eventDisplay(int event){
    eventId = event;
}

/*
 * void eventDispatch()
 *
 * System events (set directly via eventId or eventDisplay()):
 *
 *   id  Semantic              Effect                  Emitter
 *   --  --------------------  ----------------------  -----------------------
 *   0   Idle / no event       (none)                  end of each dispatch
 *   1   Board error           fade red (non-blocking)  Error() - core.ino
 *   2   Ethernet init OK      wipe green              netInitSetup() - network.ino
 *   3   Ext TCP check OK      blink white             netExtLinkStatus() OK
 *   4   Ext TCP check KO      blink orange            netExtLinkStatus() KO
 *   5   DHCP renew/rebind OK  fade green              netDhcpRenew() cases 2,4
 *   6   DHCP renew/rebind KO  fade red                netDhcpRenew() cases 1,3
 *
 * Board reset: handled directly in reset() - core.ino (brief blue fill,
 * no eventId; the board reboots 15ms later via watchdog).
 *
 * App events (set by httpRead() from the server ASCII response '0'-'9'):
 *
 *   id   char  Semantic                    Effect
 *   ---  ----  --------------------------  ------------------------------
 *   48   '0'   server error                continuous fade red
 *   49   '1'   available + vacant          wipe green
 *   50   '2'   available + occupied        wipe orange
 *   51   '3'   booked + vacant             wipe pink
 *   52   '4'   booked + occupied           wipe red
 *   53   '5'   soon in use + vacant        wipe orange
 *   54   '6'   soon in use + occupied      continuous toggle green/orange
 *   55   '7'   (user-defined)              wipe blue
 *   56   '8'   (user-defined)              wipe white
 *   57   '9'   off                         wipe black
 *
 * App events replay only when the server code changes (lastAppEvent guard).
 * All animations use the non-blocking state machine in neopixel.ino (animTick).
 */

void eventDispatch(){
  const __FlashStringHelper* eventTag = F("LED:DISP");
  switch(eventId) {
    #if VERBOSE >= 4
    case 0:
      serialMessage('i',eventTag, F("No Evt"));
    break;
    #endif
    case 1:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("Board Err"));
      #endif
      animStartFade('r', 1, FADE_SPEED, false);
    break;
    case 2:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("Eth Init OK"));
      #endif
      animStartWipe(strip.Color(40,226,0), 1, 100);
    break;
    case 3:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("TCP Check OK"));
      #endif
      animStartToggle(strip.getPixelColor(0), strip.Color(1,1,1), 50, false);
    break;
    case 4:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("TCP Check KO"));
      #endif
      animStartToggle(strip.getPixelColor(0), strip.Color(20,113,0), 100, false);
    break;
    case 5:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("DHCP OK"));
      #endif
      animStartFade('g', 1, FADE_SPEED, false);
    break;
    case 6:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("DHCP KO"));
      #endif
      animStartFade('r', 1, FADE_SPEED, false);
    break;
    /*
     * Events for ASCII server responses 
     * Replay only when app code changes (lastAppEvent guard).
     */
    static int lastAppEvent = -1;
    case 48:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("0 = server error = blinkfade red"));
      #endif
      animStartFade('r', 1, FADE_SPEED, true);
    break;
    case 49:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("1 = avail+vacant = green"));
      #endif
      animStartWipe(strip.Color(255,0,0), strip.numPixels(), WIPE_SPEED);
    break;
    case 50:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("2 = available+occupied = orange"));
      #endif
      animStartWipe(strip.Color(20,113,0), strip.numPixels(), WIPE_SPEED);
    break;
    case 51:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("3 = booked+vacant = pink"));
      #endif
      animStartWipe(strip.Color(0,81,81), strip.numPixels(), WIPE_SPEED);
    break;
    case 52:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("4 = booked+occupied = red"));
      #endif
      animStartWipe(strip.Color(0,162,0), strip.numPixels(), WIPE_SPEED);
    break;
    case 53:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("5 = soon in use + vacant = orange"));
      #endif
      animStartWipe(strip.Color(20,113,0), strip.numPixels(), WIPE_SPEED);
    break;
    case 54:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("6 = soon_in_use+occupied = blinking orange"));
      #endif
      animStartToggle(strip.Color(40,226,0), strip.Color(20,113,0), 500, true);
    break;
    case 55:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("7 = blue"));
      #endif
      animStartWipe(strip.Color(0,0,255), strip.numPixels(), WIPE_SPEED);
    break;
    case 56:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("8 = white"));
      #endif
      animStartWipe(strip.Color(255,255,255), strip.numPixels(), WIPE_SPEED);
    break;
    case 57:
      if (lastAppEvent == eventId) break;
      lastAppEvent = eventId;
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("9 = off"));
      #endif
      animStartWipe(strip.Color(0,0,0), strip.numPixels(), WIPE_SPEED);
    break;
  }
  eventId = 0;
}
