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

void httpReq() {
  client.stop();
  // reset parser state at the start of each request cycle
  httpCapturing = false;
  httpTokenPos  = 0;
  #if VERBOSE > 2
  serialMessage('d',F("HTTP:SND"),serverIP);
  #endif 
  if (client.connect(serverIP,serverPORT)) {
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
    #if VERBOSE >= 1 
    serialMessage('e',F("HTTP:SND:KO"),serverIP);
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

/*
void readHttpEvent(){
unsigned long start = micros();

const int kNetworkTimeout = 500;
const int kNetworkDelay = 100;

int statusCode = http.responseStatusCode();
unsigned long timeoutStart = millis();
if ( (statusCode >= 200) && (statusCode < 300) 
      && ((millis() - timeoutStart) < kNetworkTimeout) )
  {
    int skipHeaders = http.skipResponseHeaders();
    if (skipHeaders >= 0){
      int bodyLen = http.contentLength();
      if (bodyLen == 4){
        timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
              ((millis() - timeoutStart) < kNetworkTimeout) )
        {
          if (http.available()){
            c = http.read();
            // Print out this character
            //Serial.print(c);
            bodyLen--;
            // We read something, reset the timeout counter
            timeoutStart = millis();
          }
        }
      }
    }
  }
  http.stop();    
  unsigned long end = micros();
  unsigned long delta = end - start;
  Serial.print("FUNC11::TIME: ");
  Serial.println(delta);
}
 
void getHttpEvent(){
  unsigned long start = micros();
  http.stop();

  int statusConnect = http.get(SERVER, SERVERPORT, "/lights/0000deadbeef", "fluolight");
  if (statusConnect == 0){
    //Serial.println("startedRequest ok");
  }
  else{
    Serial.print("Connect failed: ");
    Serial.println(statusConnect);
  }
  unsigned long end = micros();
  unsigned long delta = end - start;
  Serial.print("FUNC10::TIME: ");
  Serial.println(delta);
}
*/
