/*
Connect two nails and a resistor as shown

digital 2---*
                 |
                 \
                 /
                 \ R1
                 /
                 |
                 |
analog 0----*
                 |
                 |
                 *----> nail 1
                 
                 *----> nail 2
                 |
                 |
                 |
digital 3---*
*/

#define moisture_input 0
#define divider_top A4
#define divider_bottom A5

#define CALIB_WET 980
#define CALIB_DRY 0

const unsigned long MINIMUM_TIME_BETWEEN_MOISTURE_READINGS = 5000;

long lastMoistureMillis = 0;

int SoilMoisture(){
  int reading;

  // drive a current through the divider in one direction
  digitalWrite(divider_top,HIGH);
  digitalWrite(divider_bottom,LOW);

  // wait a moment for capacitance effects to settle
  delay(1000);

  // take a reading
  reading=analogRead(moisture_input);

  // reverse the current
  digitalWrite(divider_top,LOW);
  digitalWrite(divider_bottom,HIGH);

  // give as much time in 'revers'e as in 'forward'
  delay(1000);

  // stop the current
  digitalWrite(divider_bottom,LOW);

  return reading;

}

void moistureSetup() {
  // set driver pins to outputs
  pinMode(divider_top,OUTPUT);
  pinMode(divider_bottom,OUTPUT);
}

void moistureLoop() {
  long ms = millis();
  if (ms >= lastMoistureMillis + MINIMUM_TIME_BETWEEN_MOISTURE_READINGS) {
    lastMoistureMillis = ms;
    
    lastTemperature_h = readTemp();

    int val = SoilMoisture();
    int moisture = map(val, CALIB_DRY, CALIB_WET, 0, 1000);
    Serial.print(F("Soil moisture: "));
    Serial.print(moisture/10);
    Serial.print('.');
    Serial.print(moisture%10);
    Serial.println('%');
  }
}
