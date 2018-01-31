# arduino_volt_temp_meter_sevseg

## Objective
The objective is a simple 3 digit, 7 segment display (actually 8) and a button with the capabilities to show either voltage or temperature. More specifically, I created this program so that I could put this 7 segment display inside a BMW 535is e28 check panel (should also work for other similar check panels)

## Hardware needed
A simple Atmel 328p-pu should do the trick, also needed are some pasive components, a voltage regulator (I used a 7805 with some caps), also a crystal (if using external oscilator) and some 22pf caps, a NTC thermistor, and some resistors to act as voltage divisors.

## Schematic
I won't be providing a schematic since the connections for the arduino are pretty simple, and just looking at the sketch should be enough, if you can't figure out the schematic from some googling and looking at the sketch maybe you shouldn't be cracking open your BMW Check Control :slightly_smiling_face:
