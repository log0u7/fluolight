/*
 * 
 * Serial Logging
 * 
 **/

/*
 * Write standardized messages to serial monitor.
 *
 * char type : (e)rror, (w)arning, (i)nformation, (d)ebug
 * const __FlashStringHelper* tag : stage marker (use F("..."))
 * const __FlashStringHelper* message : flash string message (use F("..."))
 * const char* message : RAM string message
 *
 * Examples:
 *   serialMessage('e', F("NET:LNK"), F("KO"));
 *   serialMessage('i', F("ETH:INIT"), someCharPtr);
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
