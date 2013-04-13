#if ENABLE_ETH



byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

#ifdef STATIC_IP
byte ip[] = STATIC_IP;
#endif



void ethernetSetup() {
  resetEthernet();
}

void ethernetLoop() {
  sevDebug(0b10011110); // 'E'
}

void resetEthernet() {
  Serial.print(F("Initializing Ethernet..."));

  
  #ifdef STATIC_IP
  Ethernet.begin(mac, ip); 
#else
  Ethernet.begin(mac);
#endif

  Serial.println(F(" OK."));

}

#else

void ethernetSetup() { }
void ethernetLoop() { }

#endif
