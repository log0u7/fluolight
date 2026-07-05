/*
 * 
 * HTTP Client
 * 
 **/
#if PROXYSET == 1
static const int serverPORT = PROXYPORT;
static const char serverIP[] = PROXY;
static const char baseUrl[]  = SERVER;                           
#else
static const int serverPORT = SERVERPORT;
static const char serverIP[] = SERVER;
#endif

// Parser state: persistent across calls to handle TCP fragmentation
static bool    httpCapturing = false;   // '<' seen, waiting for '>'
static char    httpToken[10];           // content captured between '<' and '>'
static uint8_t httpTokenPos  = 0;

// Consecutive connect failure counter; log KO only after threshold
static uint8_t httpFailCount = 0;

void httpReq() {
  client.stop();
  // reset parser state at the start of each request cycle
  httpCapturing = false;
  httpTokenPos  = 0;
  #if VERBOSE > 2
  serialMessage('d',F("HTTP:SND"),serverIP);
  #endif 
  if (client.connect(serverIP,serverPORT)) {
    httpFailCount = 0;
    #if PROXYSET == 1   
    // Craft PROXY HTTP request :
    client.print(F("GET http://"));
    client.println(baseUrl);
    client.print(F("/lights/")); 
    client.print(mac2Str(mac)); 
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: ")); 
    client.println(baseUrl);  
    #else 
    // craft DIRECT GET HTTP request:
    client.print(F("GET /lights/"));
    client.print(mac2Str(mac));
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(serverIP);  
    #endif
    client.println(F("Connection: close"));
    client.println();
    #if VERBOSE > 2
    serialMessage('d',F("HTTP:SND:OK"),serverIP);
    #endif
  } else {
    httpFailCount++;
    #if VERBOSE >= 1
    if (httpFailCount >= HTTP_FAIL_THRESHOLD) {
      serialMessage('e',F("HTTP:SND:KO"),serverIP);
    }
    #endif
  }
}

void httpRead() {
  if (client.available() <= 0) return;

  #if RESET_ON_FAIL > 0
  ErrorCount = 0;
  #endif
  #if VERBOSE > 2
  serialMessage('d',F("HTTP:RCV:OK"),ip2Str(client.remoteIP()));
  #endif

  while (client.available() > 0) {
    char c = (char)client.read();

    if (c == '<') {
      httpCapturing = true;
      httpTokenPos  = 0;
    } else if (httpCapturing) {
      if (c == '>') {
        httpToken[httpTokenPos] = '\0';
        // V1 strict: token must be exactly one digit
        if (httpTokenPos == 1 && isDigit(httpToken[0])) {
          eventId = httpToken[0];
          #if VERBOSE > 2
          serialMessage('d',F("HTTP:RCV"),httpToken);
          #endif
        }
        httpCapturing = false;
      } else if (httpTokenPos < sizeof(httpToken) - 1) {
        httpToken[httpTokenPos++] = c;
      } else {
        // token overflow: abandon current capture
        httpCapturing = false;
      }
    }
  }
}
