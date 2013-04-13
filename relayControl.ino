
const int HYSTERESIS_H = 200;
const int DEFAULT_TARGET_TEMPERATURE_H = 0;

int minOffTemp_h;
int maxOnTemp_h;

unsigned char relayStatus;

void relayControlSetup() {
  pinMode(RELAY_PIN, OUTPUT);
  
  setTargetTemperature(DEFAULT_TARGET_TEMPERATURE_H);
}

void relayControlLoop() {
  sevDebug(0b01110110); // 'Y'

  if (!temperatureOK)
    return;
    
  if (relayStatus == LOW && lastTemperature_h < minOffTemp_h) {
    Serial.println(F("Switching relay on"));
    relayStatus = HIGH;
    digitalWrite(RELAY_PIN, HIGH);
  }
  if (relayStatus == HIGH && lastTemperature_h > maxOnTemp_h) {
    Serial.println(F("Switching relay off"));
    relayStatus = LOW;
    digitalWrite(RELAY_PIN, LOW);
  }
}

void setTargetTemperature(int temp_h) {
  targetTemperature_h = temp_h;
  minOffTemp_h = targetTemperature_h - HYSTERESIS_H/2;
  maxOnTemp_h = targetTemperature_h - HYSTERESIS_H/2;
  Serial.print(F("temperature set to "));
  Serial.println(temp_h);
}
