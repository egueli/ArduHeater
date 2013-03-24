#ifdef USE_7SEG

#include <TimerOne.h>
#include <OneWire.h>

#define SDATA 7
#define SCLK 3
#define RCLK 6
#define DRES 5
#define KEYS A2

const unsigned long DISP_ALTERN_TIME = 2000;

//#define SEVENSEG_DEBUG
//#define SEVENSEG_SHOWMEM

/*
 * Warning: writing to serial in ISR may cause serial buffer
 * overruns & a lockup. Print with care.
 */
//#define ISR_SERIAL 1000

const byte DIGITS[] = {
  0b11111100,
  0b01100000,
  0b11011010,
  0b11110010,
  0b01100110,
  0b10110110,
  0b10111110,
  0b11100000,
  0b11111110,
  0b11110110
};

const byte SEVSEG_o    = 0b00111010;
const byte SEVSEG_n    = 0b00101010;
const byte SEVSEG_f    = 0b10001110;
const byte SEVSEG_DASH = 0b00000010;
const byte SEVSEG_DOT  = 0b00000001;

byte segments[] = {
  0xff, 0xff, 0xff, 0xff
};

byte lastDigit = 3;

byte debugLeds;

#ifdef ISR_SERIAL
int isrPrintCount = ISR_SERIAL;
#endif

void sevenSegISR() {
  switch(lastDigit) {
  case 0:
    sevenSegWrite(segments[1]);
    strobe(RCLK);
    lastDigit++;
    break;

  case 1:
    sevenSegWrite(segments[2]);
    strobe(RCLK);
    lastDigit++;
    break;

  case 2:
    sevenSegWrite(segments[3]);
    strobe(RCLK);
    lastDigit++;
    break;

  case 3:
#ifdef SEVENSEG_DEBUG
    sevenSegWrite(debugLeds);
#else
    sevenSegWrite(segments[0]);
#endif
    strobe(RCLK);

    strobe(DRES);
    lastDigit = 0;
    break;
  }
  
#ifdef ISR_SERIAL
  if (isrPrintCount == 0) {
    //Serial.print(F("free="));
    //Serial.print(freeMemory());
    Serial.print(F(" SP="));
    Serial.print(SP);

    Serial.print(F(" depth="));
    Serial.print(RAMEND - SP);
   
   /*
    Serial.print(F(" stack="));
    for (unsigned int p=SP; p<SP+12 && p<RAMEND; p++) {
      byte n = *((byte*)p);
      if (n<16) Serial.print('0');
      Serial.print(n, HEX);
      Serial.print(' ');
    }
    */
    
    Serial.println();
    
    isrPrintCount = ISR_SERIAL;
  }
  isrPrintCount--;
  
#endif
}

void sevenSegWrite(unsigned char leds) {
  for (byte i=0; i<8; i++) {
    digitalWrite(SDATA, leds & 1);
    strobe(SCLK);
    leds >>= 1;
  }  
}

void strobe(unsigned char pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}


void sevenSegSetup() {
  pinMode(SDATA, OUTPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(RCLK, OUTPUT);
  pinMode(DRES, OUTPUT);
  pinMode(KEYS, INPUT);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(sevenSegISR);
}

void sevenSegLoop() {
  sevDebug(0b00000000); // ' '

#ifndef SEVENSEG_SHOWMEM
  byte turn = (millis()/DISP_ALTERN_TIME) % 3;
  switch(turn) {
  case 0:
    displayTemperature();
    break;
  case 1:
    displayHeaterStatus();
    break;
  case 2:
    displayTime();
    break;
  }
#else
  displayFreeMem();
#endif
}


void displayFreeMem() {
  int t = freeMemory(); // togliere centesimi
  byte dig0 = t % 10;
  t /= 10;
  byte dig1 = t % 10;
  t /= 10;
  byte dig2 = t % 10;
  
  segments[3] = DIGITS[dig2];
  segments[2] = DIGITS[dig1];
  segments[1] = DIGITS[dig0];
  segments[0] = 0b00111110;  // "b"
}


void displayTemperature() {
  int t = lastTemperature_h / 10; // togliere centesimi
  byte tenths = t % 10;
  t /= 10;
  byte units = t % 10;
  t /= 10;
  byte tens = t % 10;
  
  segments[3] = DIGITS[tens];
  segments[2] = DIGITS[units] | SEVSEG_DOT; // punto
  segments[1] = DIGITS[tenths];
  segments[0] = 0b11000110;  // "C"
}

void displayHeaterStatus() {
  if (heaterOn) {
    segments[3] = 0b00000000;
    segments[2] = SEVSEG_o;
    segments[1] = SEVSEG_n;
  }
  else {
    segments[3] = SEVSEG_o;
    segments[2] = SEVSEG_f;
    segments[1] = SEVSEG_f;
  }
  segments[0] = 0;
}

void displayTime() {
  if (timeOK) {
    byte hour10s = hour / 10;
    segments[3] = (hour10s != 0) ? DIGITS[hour10s] : 0b00000000;
    segments[2] = DIGITS[hour % 10] | SEVSEG_DOT; // punto
    segments[1] = DIGITS[minute / 10];
    segments[0] = DIGITS[minute % 10];
  }
  else {
    segments[3] = SEVSEG_DASH;
    segments[2] = SEVSEG_DASH | SEVSEG_DOT;
    segments[1] = SEVSEG_DASH;
    segments[0] = SEVSEG_DASH;
  }
}

void sevDebug(byte leds) {
  debugLeds = leds;
}

#else // USE_7SEG
void sevDebug(byte) {}

#endif // USE_7SEG
