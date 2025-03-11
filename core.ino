/*
 * 
 * Internal Processing
 * 
 **/

/* 
 * void serialMessage() write standardized messages to serial monitor 
 * char type        : (e)rror, (w)arning, (i)nformation, (d)ebug
 * String tag       : Stage Marker, Messafe identifier
 * String message   : Message to send
 *  
 * // write : "INFO::CORE::SAMPLE: Hello World !" to serial
 * serialMessage('i', 'CORE::SAMPLE', "Hello World !") 
 *  
 */
#if VERBOSE >= 1
void serialMessage(char type, const String tag , const String message){
  switch(type){ 
    case 'e':
      Serial.print(F("ERR:")); 
      Serial.print(tag); 
      Serial.print(F(": "));
      Serial.println(message);
    break;     
    #if VERBOSE >= 2
    case 'w':
      Serial.print(F("WRN:"));
      Serial.print(tag);
      Serial.print(F(": "));
      Serial.println(message);
    break;     
    case 'i':
      Serial.print(F("INF:"));
      Serial.print(tag);
      Serial.print(F(": "));
      Serial.println(message);
    break;     
    #endif
    #if VERBOSE >= 3
    case 'd':  
      Serial.print(F("DBG:"));
      Serial.print(tag);
      Serial.print(F(": "));
      Serial.println(message);
    break;     
    #endif
  }
}
#endif 
/*
 * String mac2Str()
 * 
 * Convert Mac Array to String
 * 
 */
String mac2Str(uint8_t* in) {
  static char macStr[18];
  snprintf(macStr, sizeof(macStr),
          "%02x%02x%02x%02x%02x%02x",
          in[0], in[1], in[2], in[3], in[4], in[5]);
  String tmp = macStr;
  return tmp;    
}
/*
 * String ip2Str()
 * 
 * Convert IP Array to String
 */
String ip2Str(IPAddress ip) {
  static char ipStr[16];
  snprintf(ipStr, sizeof(ipStr),
              "%d.%d.%d.%d",
              ip[0], ip[1], ip[2], ip[3]);  
  String tmp = ipStr;
  return tmp;
}

 /*
  * Error 
  * Trigger reset function when MAX_RETRY errors is reached
  */
#if RESET_ON_FAIL > 0 
static int ErrorCount; 
void Error(const String errorDescription)
{
  #if VERBOSE >= 1
  serialMessage('e',F("FLUO:ERR:TRGR"), errorDescription);
  #endif
  ErrorCount++;
  #if VERBOSE >= 1
  serialMessage('e',F("FLUO:ERR:TRGR:COUNT"), String(ErrorCount));
  #endif
  eventId = 1;
  if (ErrorCount >= MAX_RETRY)
    ErrorCount = 0;
    reset('s', errorDescription);
}

/*
 * Reset w/ Timer
 * type :
 * s : short reset as soon as possible
 * l : long reset when REBOOT_TIMEOUT is reached
 */
void reset(char type, String errorString) {
  static int timeout = REBOOT_TIMEOUT; 
  static int partial = 1000;
  if (type == 's') {
        #if VERBOSE >= 1
        serialMessage('e',F("FLUO:SOFTRST"), errorString);
        #endif
        eventId = 2;
        void (*softReset) (void) = 0;
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
