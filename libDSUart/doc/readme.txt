DS brut Readme


Content

1. Including libDSuart in your devkitPro application
2. Modifying the firmware
3. Notes about the cartridge
4. Troubleshooting: Startup Problems
5. Troubleshooting: Debugging Hardware Problems
6. Notes about the NDS SPI Interface



1. Including libDSuart in your devkitPro application

For including the library in your own devkitPro application, copy the 
following files to your arm9 source directory: from arm9/include/ uart.h 
and from arm9/source/ spi.h, spi_driver.h, spi_internals.h and uart.c. 
Then, #include "uart.h" in your main.c file and make sure that uart.c is 
also being build, along with your own code files.



2. Modifying the firmware

The default firmware sourcecode can be found in atmega/arduino_uart.txt. 
Note: Because of changes to Arduino's Wire library, you'll need the 
latest Arduino software (0012) to compile the firmware.

Steps:
1. Get an Arduino board and carefully remove its Atmega chip
2. Wire the cartridges RX and TX pins to the Arduino ones
3. Wire the cartridge GND to Arduino GND
4. In the Arduino software, make sure Tools > Board is set to LilyPad 
   Arduino
5. When uploading to the board do a powercycle on the NDS to reset the 
   Atmega chip on the cartridge



3. Notes about the cartridge

The DS brut cartridge features one Atmega168 microcontroller running at 
8MHz and 3.3V. This means that the logic and ADC levels also have to be 
in the 0-3.3V range.
We haven't measured how much you can draw from Slot-1, but the maximum 
current per I/O pin is rated at 40mA.



4. Troubleshooting: Startup Problems

We found out that the Nintendo DS seems to react by certain auxiliary 
circuits connected to DS brut's pin headers by booting into the stock 
Nintendo DS menu instead into your homebrew one when you power the 
device on. This seems to be a general issue as DSerial2 appears to have 
the same problem.

If you end up in the stock Nintendo DS menu, try to remove the cartridge 
while powering up or simply remove the GND connection before you turn 
the Nintendo DS on and reconnect it after the homebrew menu apprears - 
this seems to do the trick for us.



5. Troubleshooting: Debugging Hardware Problems

If you are having troubles and want to verify that the cartridge is 
working as expected we suggest reading the firmware version by
calling uartSoftwareVersion(). The value you receive should be bigger 
than zero and not 0xff (255). If you get a correct response you have 
now established that:

* the timer interrupt on the NDS is working (doing SPI)
* the GND, CLK, VCC, MISO and MOSI lines are connecting the NDS and the 
  cartridge
* the Atmega168 uC is working and executing the firmware on the cartridge



6. Notes about the NDS SPI Interface

The NDS SPI interface uses the settings CPOL=1, CHPA=1. These don't seem 
to be changable via registers.