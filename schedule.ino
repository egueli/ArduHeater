#include <EEPROM.h>

const int HOT_WATER_TEMP_H = 5000;
const int COLD_WATER_TEMP_H = 1500;

const int EEADDR_HEATER_LAST_STATE = 0;

const int HEATING_BEGIN_MINS = 6 *60 + 00;
const int HEATING_END_MINS   = 9 *60 + 00;

boolean heatingBeginDone = false;
boolean heatingEndDone = false;

boolean lastTimeOKStatus = false;

void scheduleSetup() {
  /*
    Lo stato del boiler all'accensione e' l'opposto dell'ultimo stato
    che e' stato impostato
    Se il boiler era acceso prima, sara' spento.
    Se il boiler era spento prima, sara' acceso.
    Per accendere o spegnere il boiler sara' sufficiente togliere e ridare
    l'alimentazione all'Arduino.
  */
  boolean lastHeaterState = EEPROM.read(EEADDR_HEATER_LAST_STATE) > 0;
  Serial.print(F("Last heater state: "));
  Serial.println(lastHeaterState ? "on" : "off");
  turnHeater(!lastHeaterState);
}


int minutesNow() {
  return minutes(hour, minute);
}

int minutes(byte h, byte m) {
  return (int)h*60 + m;
}

boolean isScheduledHeatingTime() {
  return minutesNow() >= HEATING_BEGIN_MINS
      && minutesNow() <  HEATING_END_MINS;
}

void scheduleLoop() {
   sevDebug(0b10011100); // 'C'

  if (timeOK) {
    if (!lastTimeOKStatus) {
      /* L'orologio e' stato settato. Imposta i flag "fatto" a seconda dell'orario,
      senza effettuare alcuna azione sul boiler. */
      heatingBeginDone = minutesNow() > HEATING_BEGIN_MINS;
      heatingEndDone   = minutesNow() > HEATING_END_MINS;
    }
    else {
      doOnceInADay(&heatingBeginDone, true,  HEATING_BEGIN_MINS);
      doOnceInADay(&heatingEndDone  , false, HEATING_END_MINS  );
    }
  }
  
  lastTimeOKStatus = timeOK;
}

void doOnceInADay(boolean* doneFlag, boolean heaterStatusToSet, int minutes) {
  int nowM = minutesNow();

/*  
  Serial.print("doOnceInADay: statusToSet=");
  Serial.print(heaterStatusToSet ? "hot " : "cold");
  Serial.print(" minutes=");
  Serial.print(minutes);
  Serial.print(" done=");
  Serial.print((*doneFlag) ? "yes" : "no ");
  Serial.print(" now=");
  Serial.println(nowM);
*/

  if (!(*doneFlag) && nowM >= minutes) {
    Serial.print(F("Doing planned action at "));
    Serial.print(minutes);
    Serial.println(" minutes since midnight");
    turnHeater(heaterStatusToSet);
    *doneFlag = true;
  }
  if ( (*doneFlag) && nowM < minutes) {
    Serial.print(F("Resetting 'done' status for planned action at "));
    Serial.println(minutes);
    *doneFlag = false;
  }  
}

void turnHeater(boolean on) {
  setTargetTemperature(on ? HOT_WATER_TEMP_H : COLD_WATER_TEMP_H);
  heaterOn = on;
  EEPROM.write(EEADDR_HEATER_LAST_STATE, heaterOn ? 1 : 0);
  Serial.print(F("Heater turned "));
  Serial.println(heaterOn ? "on" : "off");
}


