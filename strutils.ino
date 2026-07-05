/*
 * 
 * String Utility Functions
 * 
 **/

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
