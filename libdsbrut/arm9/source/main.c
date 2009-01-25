#include <nds.h>
#include <stdio.h>
#include "uart.h"


int main(void)
{
	//touchPosition touch;
	uint32 pressed;
	uint32 bps = 1;
	
	consoleDemoInit();
	
	iprintf("libdsbrut test");
	iprintf("build %s %s\n\n", __DATE__, __TIME__);
	
	while(1) {

		scanKeys();
		pressed = keysDown();
		
		if (pressed & KEY_X) {
		}
		if (pressed & KEY_A) {
			iprintf("init: %u\n", uart_init());
		}
		if (pressed & KEY_B) {
			iprintf("sending.. ");
			uart_send("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
			uart_flush();
			iprintf("done\n");
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

		/*
		touchRead(&touch);
		iprintf("\x1b[10;0HTouch x = %04i, %04i\n", touch.rawx, touch.px);
		iprintf("Touch y = %04i, %04i\n", touch.rawy, touch.py);
		*/

		swiWaitForVBlank();
	}

	return 0;
}
