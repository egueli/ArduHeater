#if (ENABLE_ETH && ENABLE_PACHUBE)

#include <string.h>

const char* USERAGENT = "ArduHeater";

EthernetClient pachubeClient;
const char* pachubeServerAddr = "api.cosm.com";

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
//boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 60*1000L; //delay between updates to Pachube.com

void pachubeSetup() {

}

void pachubeLoop() {
  sevDebug(0b11001110); // 'P'
  
//  // if there's no net connection, but there was one last time
//  // through the loop, then stop the client:
//  if (!pachubeClient.connected() && lastConnected) {
//    Serial.println();
//    Serial.println("disconnecting.");
//    pachubeClient.stop();
//  }
//  
  unsigned long ms = millis();
  if(/*!pachubeClient.connected() && */(ms >= postingInterval + lastConnectionTime)) {
    lastConnectionTime = ms;
    sendData();
  }

//  lastConnected = pachubeClient.connected();

}


// this method makes a HTTP connection to the server:
void sendData() {
  // if there's a successful connection:
  Serial.print(F("trying to connect to Pachube... "));
  if (pachubeClient.connect(pachubeServerAddr, 80)) {

    // send the HTTP PUT request:
    pachubeClient.print(F("PUT /v2/feeds/"));
    pachubeClient.print(FEEDID);
    pachubeClient.println(F(".csv HTTP/1.1"));
    pachubeClient.println(F("Host: api.cosm.com"));
    pachubeClient.print(F("X-ApiKey: "));
    pachubeClient.println(APIKEY);
    pachubeClient.print(F("User-Agent: "));
    pachubeClient.println(USERAGENT);
    pachubeClient.print(F("Content-Length: "));
    
    char content[100];
    char buf[7];
    
    content[0] = '\0';
    strcat(content, "temperature,");
    numberInHundredthsToCharArray(lastTemperature_h, buf);
    strcat(content, buf);
    
    strcat(content, "\ntarget_temperature,");
    numberInHundredthsToCharArray(targetTemperature_h, buf);
    strcat(content, buf);
    
    strcat(content, "\n");    
    
    const byte thisLength=strlen(content);
    
    pachubeClient.println(thisLength, DEC);
//    Serial.print("Content-length: ");
//    Serial.println(thisLength);
    
    // last pieces of the HTTP PUT request:
    pachubeClient.println(F("Content-Type: text/csv"));
    pachubeClient.println(F("Connection: close"));
    pachubeClient.println();

    // here's the actual content of the PUT request:
    pachubeClient.print(content);
//    Serial.print(content);
  
//    Serial.println("--BEGIN RESPONSE--");
  
    boolean printHeader = true;
    while (pachubeClient.connected()) {
      if (pachubeClient.available()) {
        char c = pachubeClient.read();
        if (printHeader) {
          Serial.write(c);
          if (c == '\n') {
            printHeader = false;
          }
        }
      }
    }
  
    pachubeClient.stop();

//    Serial.println("--END RESPONSE--");  
  } 
  else {
    // if you couldn't make a connection:
    Serial.println(F("connection failed"));
    Serial.println();
    Serial.println(F("disconnecting."));
    pachubeClient.stop();
  }
}

#else

void pachubeSetup() { }

void pachubeLoop() { }

#endif

