Bootloaders


Content

1. .hex file descriptions
2. Compiling your own bootloader
3. Flashing bootloaders



1. .hex file descriptions

dsbrut_168.hex .. that's a modified version of the LilyPad Arduino 
bootloader we currently put on DS brut cartridges. It was modified 
to allow the Watchdog timer to work, see ATmegaBOOT.c and 
lilypad_watchdog.patch for details.

dsbluetooth_168.hex .. that's the compiled firmware (see 
libDSuart/atmega/arduino_uart.txt) we use for flashing DS bluetooth 
cartridges. You can also use this one if you don't want to have a 
bootloader running on your device.


2. Compiling your own bootloader

Make changes to ATmegaBOOT.c and execute "make dsbrut". Make sure 
that the toolchain (e.g. arduino-0011/hardware/tools/avr/bin) is 
inside your PATH.

Be aware that compiling bootloaders seem to be broken in 
Arduino 0012 (avr-gcc 4.3.0), avrdude outputs an error while flashing 
there. Use the Arduino 0011 toolchain (avr-gcc 4.1.2) for the time 
being.


3. Flashing bootloaders

We use the following commands

avrdude -c usbasp -p m168 -i 70 -e -u -U lock:w:0x3f:m -U efuse:w:0x00:m -U hfuse:w:0xdd:m -U lfuse:w:0xe2:m
avrdude -c usbasp -p m168 -i 70 -e -u -U flash:w:dsbrut_168.hex -U lock:w:0x0f:m

or (in case something doesn't work with DIP2 set on the uspasp for the first step)

avrdude -c usbasp -p m168 -B 10 -i 10 -e -u -U lock:w:0x3f:m -U efuse:w:0x00:m -U hfuse:w:0xdd:m -U lfuse:w:0xe2:m
avrdude -c usbasp -p m168 -i 10 -e -u -U flash:w:dsbrut_168.hex  -U lock:w:0x0f:m
