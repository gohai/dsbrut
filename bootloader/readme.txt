Bootloaders


Content

1. .hex file descriptions
2. Compiling your own bootloader
3. Flashing bootloaders
4. Updating the firmware



1. .hex file descriptions

dsbrut_168.hex .. that's a modified version of the bootloader used in the 
Arduino (0013) project, we currently put on DS brut cartridges. It was 
modified to allow for Watchdog resets, see arduino0013-watchdog.patch for 
details.

dsbrut_168_static.hex .. that's a compiled version of the current firmware 
(see libdsbrut/atmega/dsbrut_arduino.txt) without a bootloader (use this 
if you don't want to use Arduino, untested)

dsbluetooth_168.hex .. that's the compiled firmware (see 
libdsbrut/atmega/dsbluetooth_arduino.txt) we use for flashing DS bluetooth 
cartridges. (not there yet)


2. Compiling your own bootloader

Make changes to ATmegaBOOT_168.c and execute "make dsbrut". Make sure that 
the toolchain (e.g. arduino-0013/hardware/tools/avr/bin) is inside your 
PATH.

Beware that compiling bootloaders seem to be broken in Arduino 0012 and 
0013 (they use avr-gcc 4.3.0), as avrdude outputs an error while flashing 
the resulting .hex files. Use the Arduino 0011 toolchain (avr-gcc 4.1.2) 
for the time being.


3. Flashing bootloaders

We use the following commands

avrdude -c usbasp -p m168 -e -u -U lock:w:0x3f:m -U efuse:w:0x00:m -U hfuse:w:0xdd:m -U lfuse:w:0xe2:m
avrdude -c usbasp -p m168 -e -u -U flash:w:dsbrut_168.hex -U lock:w:0x0f:m

In case something doesn't work try setting the DIP switch on the usbasp for 
< 1.5 Mhz, and try using

avrdude -c usbasp -p m168 -B 10 -i 10 -e -u -U lock:w:0x3f:m -U efuse:w:0x00:m -U hfuse:w:0xdd:m -U lfuse:w:0xe2:m
avrdude -c usbasp -p m168 -i 10 -e -u -U flash:w:dsbrut_168.hex -U lock:w:0x0f:m


4. Updating the firmware

For updating the firmware (see libdsbrut/atmega/dsbrut_arduino.txt), we 
recommend installing the improved HardwareSerial for Arduino 0017 before.
see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1242466935