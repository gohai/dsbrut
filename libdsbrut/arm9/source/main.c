#include <nds.h>
#include <stdio.h>
#include "uart.h"


int main(void)
{
	uint32 bps = 1;
	
	consoleDemoInit();
	
	iprintf("libdsbrut test");
	iprintf("build %s %s\n\n", __DATE__, __TIME__);
	
	iprintf("init: %u\n\n", uart_init());
	
	while(1) {

		scanKeys();

		if (keysDown() & KEY_A) {
			uint8 test[] = { '\\', 0x00, 0xff };
			iprintf("sending.. ");
			uart_write(test, 3);
			uart_flush();
			iprintf("done\n");
		} else if (keysDown() & KEY_B) {
			iprintf("sending.. ");
			uart_send("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
			uart_flush();
			iprintf("done\n");
		} else if (keysDown() & KEY_X) {
		
		} else if (keysDown() & KEY_Y) {
		
		}
		
		if (keysHeld() & KEY_UP) {
			if (bps > 0) {
				uart_set_spi_bps(--bps);
				iprintf("bps %u\n", bps);
			}
		}
		if (keysHeld() & KEY_DOWN) {
			uart_set_spi_bps(++bps);
			iprintf("bps %u\n", bps);
		}

		if (uart_available() > 0) {
			uint8 in;
			if (uart_read(&in, 1) == 1) {
				if (in == 0x00) {
					iprintf("(0x00)");
				} else if (in == 0xff) {
					iprintf("(0xff)");
				} else {
					iprintf("%c", in);
				}
			} else {
				iprintf("\nerror: uart_read returned != 1\n");
			}
		}

		swiWaitForVBlank();
	}

	return 0;
}
