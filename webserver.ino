#if (ENABLE_ETH && ENABLE_WEBSERVER)

#define OUTPUT_JSON 1

EthernetServer server(80);

const byte MAX_REQ_LINE_LENGTH=100;

void webserverSetup() {
  server.begin();
}

void webserverLoop() {
  sevDebug(0b10110110); // 'S'  

    // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    serveHTTPRequest(client);
  }
}

const String INVALID_STRING("INVALID");

//#define PRINT_STR_INFO(s)  Serial.print("ptr=");Serial.print((int)(&s), DEC);Serial.print(" len=");Serial.println(s.length(), DEC);


const String getPathAndQueryFromRequestLine(const char* requestLine) {
//        Serial.print("Request line: ");
//        Serial.println(requestLine);
  String req(requestLine);
  
//  PRINT_STR_INFO(req)
  
 
  if (!req.startsWith("GET")) {
    return INVALID_STRING;
  }
  
  int versionPos = req.lastIndexOf("HTTP/1.");
  if (versionPos == -1) {
    return INVALID_STRING;
  }
  
  String pathAndQuery = req.substring(4, versionPos);
  pathAndQuery.trim();
  
  return pathAndQuery;
}

const String getQueryFromPAQ(const String& pathAndQuery) {
    
  int qstnMarkPos = pathAndQuery.indexOf('?');
  if (qstnMarkPos == -1) {
    return INVALID_STRING;
  }
  
  return pathAndQuery.substring(qstnMarkPos + 1);
}

void serveHTTPRequest(EthernetClient& client) {
//  Serial.println("new client");
  // an http request ends with a blank line
  boolean currentLineIsBlank = true;

  boolean isRequestLine = true;
  char requestLine[MAX_REQ_LINE_LENGTH];
  byte requestLinePos = 0;
  
  while (client.connected()) {  
    if (client.available()) {
      char c = client.read();
      if (isRequestLine == true) {
        requestLine[requestLinePos] = c;
        requestLinePos++;
        Serial.write(c);
        if (requestLinePos == MAX_REQ_LINE_LENGTH) {
          Serial.println(F("\nMaximum request line length exceeded; truncating"));
          isRequestLine = false;
          requestLinePos = 0;
        }
      }
      // if you've gotten to the end of the line (received a newline
      // character) and the line is blank, the http request has ended,
      // so you can send a reply
      if (c == '\n' && currentLineIsBlank) {
        requestLine[requestLinePos] = '\0';
        const String& paq = getPathAndQueryFromRequestLine(requestLine);


        if (paq.startsWith("/?")) {
          const String& query = getQueryFromPAQ(paq);
//          Serial.print("Query: '");
//          Serial.print(query);
//          Serial.println("'");
          if (!query.equals(INVALID_STRING)) {
            parseQueryForTempSet(query);
          }
            
        }
        
        
        serveRequest(client);
        break;
      }
      if (c == '\n') {
        // you're starting a new line
        currentLineIsBlank = true;
        isRequestLine = false;
      } 
      else if (c != '\r') {
        // you've gotten a character on the current line
        currentLineIsBlank = false;
      }
    }
  }
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
//  Serial.println("client disonnected");
}

void parseQueryForTempSet(const String& query) {
  int equalPos = query.indexOf('=');
  if (equalPos == -1) {
//    Serial.println("Equal not found");
    return;
  }
  
  String key = query.substring(0, equalPos);
  Serial.print("key=");
  Serial.println(key);
  if (key.equals("setTemp")) {
    char buf[10];
    String valueStr = query.substring(equalPos+1);
    valueStr.toCharArray(buf, 10);
    int value = atoi(buf);
    
    if (value > 1000 && value < 7000) {
  //    Serial.print("Setting target temp to ");
  //    Serial.print(value);
  //    Serial.println(" °C.");
      setTargetTemperature(value);
    }
    else {
  //    Serial.print("Invalid temperature: ");
  //    Serial.print(value);
    }    
  }
  
#ifdef USE_LCD
  else if (key.equals("setContrast")) {
    char buf[10];
    String valueStr = query.substring(equalPos+1);
    valueStr.toCharArray(buf, 10);
    int value = atoi(buf);
    setContrast(value);
  }
#endif

  else if (key.equals("heater")) {
    String valueStr = query.substring(equalPos+1);
    if (valueStr.equals("off")) {
      turnHeater(false);
    }
    else if (valueStr.equals("on")) {
      turnHeater(true);
    }
    else if (valueStr.equals("toggle")) {
      turnHeater(!heaterOn);
    }
  }
/*  
  else if (key.equals("setH")) {
    char buf[10];
    String valueStr = query.substring(equalPos+1);
    valueStr.toCharArray(buf, 10);
    hour = atoi(buf);
  }

  else if (key.equals("setM")) {
    char buf[10];
    String valueStr = query.substring(equalPos+1);
    valueStr.toCharArray(buf, 10);
    minute = atoi(buf);
  }
*/
}

void serveRequest(EthernetClient& client) {
#if OUTPUT_JSON
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.println();
  client.print(F("{\"temperature\":"));
  client.print(numberInHundredthsToString(lastTemperature_h));
  client.print(F(", \"status\":\""));
  if (heaterOn) {
    client.print(F("on"));
  }
  else {
    client.print(F("off"));
  }
  client.print(F("\",\"hour\":"));
  client.print(hour, DEC);
  client.print(F(",\"minute\":"));
  client.print(minute, DEC);
  client.print(F("}\n"));
#else
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();
  client.println(F("<!DOCTYPE HTML>"));
  client.println(F("<html><body><h2>Current time is "));
  client.print(hour, DEC);
  client.print(':');
  client.println(minute, DEC);
  client.println(F("</h2><h2>Current temperature is "));
  client.print(numberInHundredthsToString(lastTemperature_h));
  client.println(F("</h2><h2>Heater status is "));
  if (heaterOn) {
    client.println(F("on</h2><a href='?heater=off'>Turn off!</a>"));
  }
  else {
    client.println(F("off</h2><a href='?heater=on'>Turn on!</a>"));
  }
  client.print(F("</body></html>"));
#endif
}

#else

void webserverSetup() { }
void webserverLoop() { }

#endif

