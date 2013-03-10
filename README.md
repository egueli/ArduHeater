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
* No buttons/switches/etc in the controller, because the heater is mounted ca. 2.5m from floor, therefore they would be unreachable

The solution
------------
My solution is a controller with these components:

* An Arduino with Ethernet shield
* A Wi-fi router I had lying around
* A temperature sensor put in contact with the boiler's thermostat wire
* A 7-segment display from an old project I had lying around

### The software
In this repo you will find the Arduino sketch files needed to run the controller.
