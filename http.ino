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
 
void httpReq() {
  client.stop();
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
  int len = client.available();
  if (len > 0) {
    #if RESET_ON_FAIL > 0
    ErrorCount = 0;
    #endif
    #if VERBOSE > 2
    serialMessage('d',F("HTTP:RCV:OK"),ip2Str(client.remoteIP()));
    #endif
    char buffer[160];
    if (len > 160) len = 160;
    client.read(buffer, len);
    #if VERBOSE > 2
    serialMessage('d',F("HTTP:RCV"),F("BFR:"));
    Serial.write(buffer, len);
    Serial.println();
    #endif
    for (int i = 1; i < len; i++){
      if (buffer[i] == '>'){
        if ( isDigit(buffer[i-1]) && buffer[i-1] != 0 ){
          eventId = buffer[i-1];
        }
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
