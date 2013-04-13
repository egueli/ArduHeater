/*
 * ArduHeater - a programmable, Web-enabled heater controller
 * (C) Enrico Gueli 2012
 */
 
/*
   Before compiling this sketch for the first time, do this:

   1- edit the Config.info.template to suit your needs;
   2- save as Config.ino (i.e. remove the .template extension)
   3- close and reopen this sketch.
   
   We need to do this because the constants in Config are not
   meant to be distributed freely (e.g. the Cosm API key), so
   the Config.ino file should NOT be in the repo.
   If you want to fork ArduHeater, the .gitignore will tell Git
   not to deal Config.ino.
*/

#define ENABLE_ETH 1

// depends: ENABLE_ETH
#define ENABLE_NTP 1

// depends: ENABLE_ETH
#define ENABLE_PACHUBE 1

// depends: ENABLE_ETH
#define ENABLE_WEBSERVER 1

#define USE_7SEG
//#define USE_LCD

#include <SPI.h>         
#include <Ethernet.h>
#include <EthernetUdp.h>

/*
 * This project is subdivided in modules; each module has its own
 * setup() and loop() functions. In this file, there are the two main
 * Arduino functions and any variable shared by two or more
 * modules.
 */


// Valore temporale (secondi da Epoch) dell'ultima sincronizzazione
unsigned long lastSyncTime = 0;
// Uptime (valore di millis()) quando e' avvenuta l'ultima sincr.
unsigned long lastSyncMillis = 0;

byte hour;
byte minute;
boolean timeOK = false;

boolean heaterOn = false;

boolean temperatureOK = false;
int lastTemperature_h;
int targetTemperature_h;

void setup() 
{
  serialSetup();

#ifdef USE_LCD
  lcdSetup();
#endif

#ifdef USE_7SEG
  sevenSegSetup();
#endif

  ethernetSetup();  
  timeSetup();
  webserverSetup();
  ds18s20Setup();
  relayControlSetup();
  scheduleSetup();
  pachubeSetup();
}

void loop()
{
  serialLoop();
  
  ethernetLoop();
  
  timeLoop();

  webserverLoop();  

  ds18s20Loop();

  relayControlLoop();
  
  scheduleLoop();
  
#ifdef USE_LCD
  lcdLoop();
#endif
  
#ifdef USE_7SEG
  sevenSegLoop();
#endif

  pachubeLoop();
}




  
