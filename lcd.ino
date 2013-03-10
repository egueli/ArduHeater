#ifdef USE_LCD

#include <LiquidCrystal.h>
#include <phi_big_font.h>

//#define BIG_DISPLAY

const byte DEFAULT_CONTRAST = 7;

const byte D7_PIN = 5;
const byte D6_PIN = 7;
const byte D5_PIN = 3;
const byte D4_PIN = 6;
const byte RS_PIN = 8;
const byte EN_PIN = 9;
const byte KEYS_PIN = A2;
const byte CONTRAST_PIN = A3;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(RS_PIN, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

void lcdSetup() {
  
  pinMode(CONTRAST_PIN, OUTPUT);  
  setContrast(DEFAULT_CONTRAST);

  lcd.begin(16, 2);
#ifdef BIG_DISPLAY
  init_big_font(&lcd);
#else
  lcd.print(F("ArduHeater"));
#endif
}

void lcdLoop() {
  delay(100);
#ifndef BIG_DISPLAY
  lcd.clear();

  lcd.setCursor(8, 1);
  lcd.print(getFreeMem());

  lcd.setCursor(0, 0);
  lcd.print(millis()/1000);
  lcd.print('.');
  lcd.print((millis()/100)%10);

  String row = String(" T=");
  row += numberInHundredthsToString(lastTemperature_h);
  lcd.print(row);

//  lcd.print(lastTemperature_h);

  lcd.setCursor(0, 1);
  lcd.print(hour);
  lcd.print(':');
  lcd.print(minute);
  lcd.print(' ');

//  lcd.print(Ethernet.localIP());
  
#else

  render_big_number(lastTemperature_h/100, 0, 0);
  render_big_number((lastTemperature_h/10)%10, 9, 0);
  render_big_number((lastTemperature_h%10), 13, 0);
//  render_big_char('°', 13, 0);
//  lcd_clear();
//  render_big_char('A', 0, 0);
#endif
}

void setContrast(byte contrast) {
  digitalWrite(CONTRAST_PIN, HIGH);
  writeHighNibble(contrast);
  digitalWrite(CONTRAST_PIN, LOW);
}

void writeHighNibble(byte num) {
  byte c3 = (num & 8) ? HIGH : LOW;
  byte c2 = (num & 4) ? HIGH : LOW;
  byte c1 = (num & 2) ? HIGH : LOW;
  byte c0 = (num & 1) ? HIGH : LOW;
  digitalWrite(D7_PIN, c3);
  digitalWrite(D6_PIN, c2);
  digitalWrite(D5_PIN, c1);
  digitalWrite(D4_PIN, c0);
}

#endif // USE_LCD
