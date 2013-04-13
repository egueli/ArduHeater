
const int EEADDR_LAST_SYNC_TIME = 2;

const unsigned long EE_TIME_UPDATE_INTERVAL = 60 * 1000UL;

#if (ENABLE_ETH && ENABLE_NTP)
#define _ENABLE_NTP 1
#endif

#if _ENABLE_NTP

unsigned int localPort = 8888;      // local port to listen for UDP packets
IPAddress timeServer(193, 204, 114, 105); // time.ien.it NTP server

// A UDP instance to let us send and receive NTP data
EthernetUDP Udp;


const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

const unsigned long NTP_QUERY_INTERVAL = 10 /* minutes */ *60*1000UL;

//const byte MAX_ERRORS_BEFORE_ETH_RESET = 3;

byte errors = 0; 
// valore di millis() al momento dell'ultima query effettuata
unsigned long lastNtpQueryTime_ms;
// valore di tempo Epoch ritornato dall'ultima query
unsigned long lastNtpTime_s = -1;

#else

#endif



unsigned long lastEepromUpdateTime_ms = 0;


unsigned long wallTime() {
  return lastSyncTime + (millis() - lastSyncMillis) / 1000;
}

void updateWallClock() {  
  unsigned long wt = wallTime();
  minute = (wt  % 3600) / 60;
  hour = (wt  % 86400L) / 3600;
  timeOK = true;
}


union myLong {
  unsigned long anUnsignedLong;
  byte fourBytes[4];
};

void setClockFromEeprom() {
  union myLong st;
  st.fourBytes[0] = EEPROM.read(EEADDR_LAST_SYNC_TIME);
  st.fourBytes[1] = EEPROM.read(EEADDR_LAST_SYNC_TIME+1);
  st.fourBytes[2] = EEPROM.read(EEADDR_LAST_SYNC_TIME+2);
  st.fourBytes[3] = EEPROM.read(EEADDR_LAST_SYNC_TIME+3);
  
  setClock(st.anUnsignedLong);
}

void setClock(unsigned long newWallTime) {
  lastSyncTime = newWallTime;
  lastSyncMillis = millis();
  Serial.print(F("Wall clock set to "));
  Serial.println(lastSyncTime);
  updateWallClock();
}

void setClock(byte newH, byte newM) {
  setClock(((unsigned long)newH)*3600 + ((unsigned long)newM)*60);
}

void updateEepromTime() {
  Serial.print(F("Saving time to EEPROM: "));
  union myLong st;
  st.anUnsignedLong = wallTime();
  Serial.println(st.anUnsignedLong);
  EEPROM.write(EEADDR_LAST_SYNC_TIME  , st.fourBytes[0]);
  EEPROM.write(EEADDR_LAST_SYNC_TIME+1, st.fourBytes[1]);
  EEPROM.write(EEADDR_LAST_SYNC_TIME+2, st.fourBytes[2]);
  EEPROM.write(EEADDR_LAST_SYNC_TIME+3, st.fourBytes[3]);
}

void timeSetup() {
  setClockFromEeprom();
  
#if _ENABLE_NTP
  Udp.begin(localPort);
#endif
}

void timeLoop() {
  sevDebug(0b00011110); // 't'
  
  unsigned long ms = millis();
  if (ms >= lastEepromUpdateTime_ms + EE_TIME_UPDATE_INTERVAL) {
    lastEepromUpdateTime_ms = ms;
    updateEepromTime();
  }
  
    
#if _ENABLE_NTP
  if (ms >= lastNtpQueryTime_ms + NTP_QUERY_INTERVAL) {
    lastNtpQueryTime_ms = ms;
    doNTPQuery();    
  } // NTP query task
  
  if (lastNtpTime_s != -1) {
    unsigned long secsSinceLastNTP = (ms - lastNtpQueryTime_ms) / 1000; /* beware to rollover! */
    unsigned long secsSince1900 = lastNtpTime_s + secsSinceLastNTP;
    lastSyncTime = secsSince1900 - 2208988800UL /* 70 years */;
    lastSyncMillis = millis();
  }
  
#endif

  updateWallClock();
}


#if _ENABLE_NTP

void doNTPQuery() {
  Serial.print(F("Reading NTP time... "));
  unsigned long ntpTime = getNTPTime();
  if (ntpTime != -1) {
    errors = 0;
    
    // do UTC to local time conversion
    lastNtpTime_s = ntpTime + TIME_ZONE_DIFFERENCE;
    
    Serial.println(F("OK."));
  }
  else {
    Serial.println(F("got no data from NTP server :("));
/*
    errors++;
    if (errors == MAX_ERRORS_BEFORE_ETH_RESET) {
      Serial.println("resetting Ethernet");
      resetEthernet();
      Udp.begin(localPort);
      errors = 0;
    }
*/
  }

}

unsigned long getNTPTime() {
  sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
  delay(1000);  
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    return secsSince1900;
  }
  else
    return -1;
}


// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
//  Serial.println("sending NTP request");
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
//  Serial.println("NTP request sent");
}

#endif
