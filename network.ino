/*
 * 
 * Networking Functions
 *
 **/
 
#if DHCP == 0
static const IPAddress ip(IP_ADDRESS);
static const IPAddress subnet(IP_SUBNET);
static const IPAddress gateway(IP_GATEWAY);      
static const IPAddress dns(IP_DNS);     
#endif 
#if EXT_LINK_CHECK < 2
static uint8_t ExternalLinkIsActive;
#endif
#if EXT_LINK_CHECK == 1   
static const IPAddress extLinkIp(EXT_LINK_SERVER);
static const int extLinkPort = EXT_LINK_PORT;
#endif

/*
 * Ititial Networking Setup
 */
void netInitSetup(){
  #if MACSET == 0
  #if VERBOSE > 1
  serialMessage('i',F("ETH"), F("GETMAC"));
  #endif
  static uint8_t mac_mem[6];
  memory.begin();
  memory.readBlock(ADDR_MAC, mac_mem, 6);
  mac = mac_mem;
  #endif
  #if VERBOSE > 1
  serialMessage('i',F("ETH:INIT"), mac2Str(mac));
  #endif
  // Note: on the fluo:avr core, hardwareStatus() returns 0 even when
  // the Ethernet chip is present and functional. The condition == 0
  // is intentional for this core (verified on hardware).
  if (Ethernet.hardwareStatus() == 0) {
  eventId = 4; eventDispatch();
    #if DHCP == 1
    #if VERBOSE > 1
    serialMessage('i',F("ETH:INIT"), F("DHCP"));
    #endif
    if (Ethernet.begin(mac) == 0) {
      #if VERBOSE > 0
      serialMessage('e',F("ETH:INIT:DHCP"), F("KO"));
      #endif
      #if RESET_ON_FAIL == 2
      reset('l', F("ETH:INIT:DHCP: KO"));
      #endif
    }
    #else
    #if VERBOSE > 1
    serialMessage('i',F("ETH::INIT"), F("MANUAL"));
    #endif
    Ethernet.begin(mac, ip, dns, gateway, subnet);
    #endif
    #if VERBOSE > 1
    //serialMessage('i',F("ETH::MAC"), mac2Str(mac));
    serialMessage('i',F("ETH::IP"),  ip2Str(Ethernet.localIP()));
    serialMessage('i',F("ETH::GW"),  ip2Str(Ethernet.gatewayIP()));
    serialMessage('i',F("ETH::DNS"), ip2Str(Ethernet.dnsServerIP()));
    #endif
  }
}

/*
 * Check Ethernet Link Status
 */
void netLinkStatus() {
  static auto Blink = true;
  if (Ethernet.linkStatus() != LinkON) {
    isEthLinkActive = 0;
    #if RESET_ON_FAIL > 0
    Error(F("ETH:LNK:KO"));
    #else
    #if VERBOSE > 0
    serialMessage('e',F("ETH:LNK"), F("KO"));
    #endif // VERBOSE >= 2
    #endif
  } else {
    isEthLinkActive = 1;
    // Blink LED (Emulate Network card LINK)
    digitalWrite(LED_BUILTIN, Blink);
    Blink = !Blink;
  }
}

/*
 * Check External TCP port
 */
#if EXT_LINK_CHECK < 2
void netExtLinkStatus() {
  #if EXT_LINK_CHECK == 0
  if (client.connect(Ethernet.dnsServerIP(), 53))
  #else
  if (client.connect(extLinkIp, extLinkPort))
  #endif
  {
    #if VERBOSE >= 2
    serialMessage('i',F("ETH:EXT_LNK"), F("OK"));
    #endif
    eventId = 6;
    ExternalLinkIsActive = 1;
    #if RESET_ON_FAIL > 0
    ErrorCount = 0;
    #endif
    #if WATCHDOG == 1
    wdt_reset();
    #endif
  } else {
    #if RESET_ON_FAIL >= 2
    Error(F("ETH:EXT_LNK: KO"));
    #endif
    #if VERBOSE >= 1
    serialMessage('e',F("ETH:EXT_LNK"), F("KO"));
    #endif 
    ExternalLinkIsActive = 0;
    eventId = 7;
  }
  client.stop();  // release socket immediately after test
}
#endif

/*
 *  Renew DHCP leases
 */
#if DHCP == 1
void netDhcpRenew() {
  #if VERBOSE >= 2
  serialMessage('i',F("ETH:DHCP"), F("RENEW"));
  #endif
  switch (Ethernet.maintain()) {
    case 1:
      #if RESET_ON_FAIL >= 2
      Error(F("ETH:DHCP:RENW: KO"));
      #else
      #if VERBOSE >= 1
      serialMessage('e',F("ETH:DHCP:RENW"), F("KO"));
      #endif
      eventDisplay(8); 
      #endif 
    break;
    case 2:
      #if VERBOSE >= 2
      serialMessage('i',F("ETH:DHCP:RENW"), F("OK"));
      #endif
      eventDisplay(7);     
    break;
    case 3:
      #if RESET_ON_FAIL >= 2
      Error(F("DHCP:REBND: KO"));
      #else
      #if VERBOSE >= 1
      serialMessage('e',F("ETH:DHCP:REBND"), F("KO"));
      #endif
      eventDisplay(8); 
      #endif 
    break;
    case 4:
      #if VERBOSE >= 2
      serialMessage('i',F("ETH:DHCP:REBND"), F("OK"));
      #endif
      eventDisplay(7);
    break;
  }
}
#endif
