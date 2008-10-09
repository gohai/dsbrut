### Uploading Atmega Code to run on the Cartridge ###

Take an Arduino board and remove its Atmega uC. Connect RX (digital 0) to TX on the cartridge and vice versa. Also connect both GNDs. Power cycle the NDS to reset the Atmega on the cartridge for uploading.


### Connecting via UART ###

Use 57600 as baudrate.


### Troubleshooting ###

NDS boots the game menu: Disconnect GND for booting.


### About the NDS SPI Interface ###

CPOL=1, CHPA=1 :/