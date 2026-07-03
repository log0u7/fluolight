/*
 * 
 * Internal Processing
 * 
 **/

/* 
 * void serialMessage() write standardized messages to serial monitor 
 * char type        : (e)rror, (w)arning, (i)nformation, (d)ebug
 * const __FlashStringHelper* tag     : stage marker (use F("..."))
 * const __FlashStringHelper* message : message (use F("...") or const char*)
 *  
 * // write : "INFO::CORE::SAMPLE: Hello World !" to serial
 * serialMessage('i', 'CORE::SAMPLE', "Hello World !") 
 *  
 */
#if VERBOSE >= 1
static void _serialMessageHeader(char type, const __FlashStringHelper* tag) {
  switch(type) {
    case 'e': Serial.print(F("ERR:")); break;
    #if VERBOSE >= 2
    case 'w': Serial.print(F("WRN:")); break;
    case 'i': Serial.print(F("INF:")); break;
    #endif
    #if VERBOSE >= 3
    case 'd': Serial.print(F("DBG:")); break;
    #endif
  }
  Serial.print(tag);
  Serial.print(F(": "));
}
void serialMessage(char type, const __FlashStringHelper* tag, const __FlashStringHelper* message) {
  _serialMessageHeader(type, tag);
  Serial.println(message);
}
void serialMessage(char type, const __FlashStringHelper* tag, const char* message) {
  _serialMessageHeader(type, tag);
  Serial.println(message);
}
#endif 
/*
 * const char* mac2Str()
 * 
 * Convert Mac Array to string (static buffer, valid until next call)
 * 
 */
const char* mac2Str(uint8_t* in) {
  static char macStr[13];
  snprintf(macStr, sizeof(macStr),
          "%02x%02x%02x%02x%02x%02x",
          in[0], in[1], in[2], in[3], in[4], in[5]);
  return macStr;
}
/*
 * const char* ip2Str()
 * 
 * Convert IP Array to string (static buffer, valid until next call)
 */
const char* ip2Str(IPAddress ip) {
  static char ipStr[16];
  snprintf(ipStr, sizeof(ipStr),
              "%d.%d.%d.%d",
              ip[0], ip[1], ip[2], ip[3]);
  return ipStr;
}

 /*
  * Error 
  * Trigger reset function when MAX_RETRY errors is reached
  */
#if RESET_ON_FAIL > 0 
static int ErrorCount;
void Error(const __FlashStringHelper* errorDescription)
{
  #if VERBOSE >= 1
  serialMessage('e', F("FLUO:ERR:TRGR"), errorDescription);
  #endif
  ErrorCount++;
  #if VERBOSE >= 1
  char countStr[6];
  itoa(ErrorCount, countStr, 10);
  serialMessage('e', F("FLUO:ERR:TRGR:COUNT"), countStr);
  #endif
  eventId = 1;
  if (ErrorCount >= MAX_RETRY) {
    ErrorCount = 0;
    reset('s', errorDescription);
  }
}

/*
 * Reset w/ Timer
 * type :
 * s : short reset as soon as possible
 * l : long reset when REBOOT_TIMEOUT is reached
 */
void reset(char type, const __FlashStringHelper* errorString) {
  static int timeout = REBOOT_TIMEOUT; 
  static int partial = 1000;
  if (type == 's') {
        #if VERBOSE >= 1
        serialMessage('e',F("FLUO:SOFTRST"), errorString);
        #endif
        // Brief blue flash so the user sees the board is rebooting.
        // Blocking here is intentional: the reset happens 15ms later anyway.
        strip.fill(strip.Color(0, 0, 255)); // GRB: G=0, R=0, B=255
        strip.show();
        delay(300);
        wdt_enable(WDTO_15MS);
        while (1) {}
  }
  if (type == 'l') {
    #if VERBOSE >= 1
    serialMessage('e',F("FLUO:SOFTRST:TRGR"), errorString);
    #endif
    while (timeout / partial > 1){
      timeout = timeout - partial;
      #if VERBOSE >= 1
      Serial.print(F("ERR:FLUO:SOFTRST:TIMR: "));
      Serial.println(timeout);
      #endif
      delay(partial);
    }
    reset('s', errorString);
  }
}
#endif
