ArduHeater
==========

An Arduino-enabled controller of an electric water heater.

Why
---
* We have an electric heater in the bathroom
* It needs ca. 50 minutes to heat up
* We want hot water available when we wake up
* We want to turn on the heater when we're not at home, via Internet
* We want to know the temperature more precisely than the simple on/off of that red lamp
* (future) We want to control the target temperature too (e.g. hot at morning, warm in daytime, cold at night)

Challenges
----------
* Less cables as possible, so the controller must stay near the heater
* No buttons/switches/etc in the controller, because the heater is mounted ca. 2.5m from floor,
  therefore they would be unreachable. A wall-mounted switch gives power to both the heater and the controller.

The solution
------------
My solution is a controller with these features:

* Temperature sensing and display
* Heater/temperature control via a relay
* Wi-Fi interface
* Wall clock synchronization via NTP
* Scheduling
* HTTP (JSON) server
* Periodic upload of temperature to Cosm.com

Hardware components:

* An Arduino with Ethernet shield
* A Wi-fi router I had lying around
* A temperature sensor put in contact with the boiler's thermostat wire
* A 7-segment display from an old project I had lying around
* A 12V relay to control the heater's power

To overcome the lack of buttons, the heater keeps an internal state (on/off) which is toggled at reset. If you
want to directly turn on/off the heater, you toggle the switch *twice*. Here is how it works:

1. If the switch is off, both the controller and the heater are off. Of course, no scheduling is possible since
the controller has no power.
2. Turning the switch on, the controller turns on the heater and saves the "on" state into its non-volatile memory.
3. Turning the switch off, everything is off again.
4. Turning the switch on again, the controller sees the previous "on" state and toggles it. The controller is on and
   the heater is off. In this state (and in step 2 too), scheduling and Internet control can happen. For each heater
   state change, the non-volatile memory is updated so the heater state is always the inverse of the state when the
   controller was turned off.

### The software
In this repo you will find the Arduino sketch files needed to run the controller.

The main file is ArduinoDoesEverything.ino. It is called this way because initially I thought to split
some functionality in the Arduino, and some other in the Wifi router. Then I managed to put almost everything in the
Arduino's little RAM and flash.
