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
 * switch static int eventIndex :
 * 0-2 System Status Events
 * 0    -> Idle / No Event
 * 1    -> Board Error 
 * 2    -> Board Soft Reset
 * 3    -> 
 * 4-9 Ethernet Status Events
 * 4/5  -> Ethernet Connecting 
 * 6/7  -> External TCP checks 
 * 8/9  -> DHCP Renew Status
 * 48-57 App Events : ASCII char '0'-'9' (server response codes)
 */

void eventDispatch(){
  const __FlashStringHelper* eventTag = F("LED:DISP");
  uint16_t i;
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
      colorFade('r',1,FADE_SPEED);
    break;
    case 2:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("Board Rst"));
      #endif
      colorFade('b',1,FADE_SPEED);
    break;
    case 4:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("Eth Up"));
      #endif
      colorWipe(strip.Color(40,226,0),1,100);
      colorBlink(strip.Color(20,113,0),1,500);
    break;
    case 5:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("Eth Manual"));
      #endif
      colorFade('b',1,FADE_SPEED);
    break;
    case 6:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("TCP Check OK"));
      #endif
      colorBlink(strip.Color(1,1,1),1,50);
    break;
    case 7:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("TCP Check KO"));
      #endif
      colorBlink(strip.Color(20,113,0),1,100);   
    break;
    case 8:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("DHCP STATUS OK"));
      #endif
      colorFade('g',1,FADE_SPEED);
    break;
    case 9:
      #if VERBOSE >= 2
      serialMessage('i',eventTag, F("DHCP STATUS K0"));
      #endif
      colorFade('r',1,FADE_SPEED);
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
