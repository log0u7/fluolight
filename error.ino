/*
 * 
 * Error Handling & Soft Reset
 * 
 **/

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
