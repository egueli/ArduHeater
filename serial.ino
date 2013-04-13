#include <memory.h>

#define CMDBUF_DEBUG 0
#define SERIAL_ECHO 1

void serialSetup() {
  Serial.begin(57600);  
  Serial.println(F("*** ArduHeater, (C) 2013 Enrico Gueli <enrico.gueli@gmail.com> ***"));
  resetCmdBuf();
}

const byte CMDBUF_LEN = 10;

char cmdBuf[CMDBUF_LEN];
byte ptr = 0;

boolean serialEnabled = true;

void serialLoop() {
  sevDebug(0b00011100); // 'L'

  if (!serialEnabled)
    return;
  
  char ch = Serial.read();  
  if (ch == -1) 
    return;

#if SERIAL_ECHO
  Serial.print(ch);
#endif

#if CMDBUF_DEBUG
  Serial.print("ptr=");
  Serial.print(ptr, DEC);  
  Serial.print(" ch=");
  Serial.print(ch, DEC);
  Serial.print(" '");
  Serial.print(ch);
  Serial.print(F("' cmdBuf=["));
  for (byte i=0; i<CMDBUF_LEN; i++) {
    Serial.print(cmdBuf[i], HEX);
    Serial.print(' ');
  }
  Serial.println(']');
#endif
  
  if (ptr == 0) {
    if (ch == 'A') {
      cmdBuf[ptr++] = ch;
    }
  }
  else if (ptr == 1) {
    if (ch == 'T')
      cmdBuf[ptr++] = ch;
    else
      resetCmdBuf();
  }
  else if (ptr >= 2 && ptr < CMDBUF_LEN) {
    if (ch == '\r' || ch == '\n') {
      cmdBuf[ptr] = 0;
      parseCommand();
    }
    else
      cmdBuf[ptr++] = ch;
  }
  else if (ptr >= CMDBUF_LEN) {
    Serial.println(F("cmdbuf full!"));
    resetCmdBuf();
  }
}

void parseCommand() {
  String cmd = String(cmdBuf);
  
#if CMDBUF_DEBUG
  Serial.print("cmd len: ");
  Serial.println(strlen(cmdBuf));
#endif
  
  if (cmd.startsWith("AT")) {
    boolean ok;
    if (cmd.length() == 2) {
      ok = true;
    }
    else if (cmd.length() > 2) {
      switch (cmd[2]) {
      case 'H':
        ok = parseHeaterCommand(cmd);
        break;
      case 'M':
        ok = parseTimeCommand(cmd);
        break;
      }
    }

    if (ok)
      Serial.println("OK");

  }
  resetCmdBuf();
}

boolean parseHeaterCommand(String& cmd) {
  if (cmd.length() < 4)
    return false;
    
  switch(cmd[3]) {
  case '0':
    turnHeater(false);
    break;
  case '1':
    turnHeater(true);
    break;
  case '?':
    Serial.println(heaterOn ? '1' : '0');
    break;
  default:
    return false;
  }
  return true;
}

boolean parseTimeCommand(String& cmd) {
  if (cmd[3] == '?') {
    Serial.print(hour, DEC);
    Serial.print(':');
    if (minute < 10) Serial.print('0');
    Serial.println(minute, DEC);
    Serial.print(F("wallTime(): "));
    Serial.println(wallTime());
    Serial.print(F("lastSyncTime: "));
    Serial.println(lastSyncTime);
    Serial.print(F("lastSyncMillis: "));
    Serial.println(lastSyncMillis);
    return true;
  }
  else {
    char charr[20];
    cmd.toCharArray(charr, 20);
    unsigned int timeCmd = atoi(charr+3);
    byte h = timeCmd / 100;
    byte m = timeCmd % 100;
    if (h >= 23 || m >= 60) {
      Serial.println(F("INVALID TIME"));
      return false;
    }
    setClock(h, m);
    return true;
  }
}


void resetCmdBuf() {
  memset(cmdBuf, 0, CMDBUF_LEN);
  ptr = 0;
}
