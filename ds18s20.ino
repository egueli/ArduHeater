#include <OneWire.h>

OneWire ds(A0);

const unsigned long MINIMUM_TIME_BETWEEN_TEMP_READINGS = 5000;

unsigned long lastTemperatureMillis = 0;


void ds18s20Setup() {
}


void ds18s20Loop() {
  sevDebug(0b01111010); // 'd'

  unsigned long ms = millis();
  if (ms >= lastTemperatureMillis + MINIMUM_TIME_BETWEEN_TEMP_READINGS) {
    lastTemperatureMillis = ms;
    
    lastTemperature_h = readTemp();
    temperatureOK = true;
/*
    Serial.print("Temperature: ");
    Serial.println(lastTemperature_h);
*/
  }
}


int readTemp() {
  byte addr[8];
  byte i;
  
  memset(addr, 0, 8);
  ds.reset_search();
  if ( !ds.search(addr)) {
    Serial.println("NO DEVICES");
    delay(250);
    return NAN;
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println(F("INVALID CRC"));
    return NAN;
  }
 
  // the first ROM byte indicates which chip
  byte type_s;
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      Serial.println(F("NO DS18X20 DEVICE"));
      return NAN;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  byte data[9];
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // convert the data to actual temperature

  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
  }
/*
  Serial.print("Raw-Temp:    ");
  Serial.print(raw);
  Serial.print("  type_s: ");
  Serial.print(type_s, DEC);
  Serial.print("  data[6, 7]: ");
  Serial.print(data[6], HEX);
  Serial.print(" ");
  Serial.println(data[7], HEX);
*/
  int celsius_h = (long)raw * 100L / 16L;
  return celsius_h;
}

String numberInHundredthsToString(int num_h) {
  char out[7];

  numberInHundredthsToCharArray(num_h, out);
  
  return String(out);
}

void numberInHundredthsToCharArray(int num_h, char* out) {
  itoa(num_h, out, 10);
  
  if (num_h < 0) num_h = -num_h;

  int len = strlen(out); // punta al null-terminator
  
  if (num_h < 10) {
    /* inserisce "0.0" fra il segno e la cifra */
    out[len+3] = '\0';
    out[len+2] = out[len-1];
    out[len+1] = '0';
    out[len  ] = '.';
    out[len-1] = '0';
  }
  else if (num_h < 100) {
    /* inserisce "0." fra il segno e le due cifre */
    out[len+2] = '\0';
    out[len+1] = out[len-1];
    out[len  ] = out[len-2];
    out[len-1] = '.';
    out[len-2] = '0';
  }
  else {
    /* inserisce un punto fra la terzultima e la penultima cifra. */
    out[len+1] = '\0';
    out[len  ] = out[len-1];
    out[len-1] = out[len-2];
    out[len-2] = '.';
  }
}

