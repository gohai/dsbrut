/*
 *	NDS UART Test Application
 *
 *	Copyright Gottfried Haider & Gordan Savicic 2008.
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "nds.h"
#include <nds/arm9/console.h>
#include <stdio.h>
#include "uart.h"
#include "reboot.h"


static const unsigned int uart_speeds[] = { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200 };


int main(void)
{
	bool high = false;
	int cur_speed = 0x09;
	touchPosition touch = { 0 };

	irqInit();
	irqEnable(IRQ_VBLANK);
	
	videoSetMode(0);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG);

	SUB_BG0_CR = BG_MAP_BASE(31);

	BG_PALETTE_SUB[255] = RGB15(31,31,31);

	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);

	iprintf("libDSUart test\n");
	iprintf("build %s %s\n\n", __DATE__, __TIME__);

	iprintf("initializing UART.. ");
	if (uartOpen())
		iprintf("successful.\n\n");
	else
		iprintf("failed!\n\n");
	
	iprintf("\nUsage:");
	iprintf("\n\n\t[A] .. send test string");
	iprintf("\n\t[B] .. cycle baud rates");
	iprintf("\n\t[Y] .. read software version");
	iprintf("\n\t[X] .. exit");
	iprintf("\n\n\t[\x11] .. digital read on PC5");
	iprintf("\n\t[\x10] .. analog read on PC5");
	iprintf("\n\t[\x1e] .. set pins high/low");
	iprintf("\n\nUse the stylo to do PWM on PD5\n\n");

	while(true) {

		char c;
		uint32 pressed;
		
		scanKeys();
		pressed = keysDown();
		
		if (keysHeld() & KEY_TOUCH) {
				// TODO: we should clear the output buffer here
				touch = touchReadXY();
				uartAnalogWrite(PD5, touch.px);
				iprintf("\nPD5 @ %3u", touch.px);
		}
		
		if (pressed & KEY_A) {
			iprintf("\n\n\tsending test string.. ");
			iprintf("%u send\n", uartPutStr("THE QUICK BROWN DOG JUMPS OVER THE LAZY NOVO\r\n"));
		} else if (pressed & KEY_B) {
			if (++cur_speed == 11)
				cur_speed = 0;
			uartSetBaud(uart_speeds[cur_speed]);
			iprintf("\n\n\tsetting baud rate to %u\n", uart_speeds[cur_speed]);
		} else if (pressed & KEY_LEFT) {
			iprintf("\ndigital read: ");
			if (uartDigitalRead(PC5))
				iprintf("high");
			else
				iprintf("low");
		} else if (pressed & KEY_RIGHT) {
			iprintf("\nanalog read: %u", uartAnalogRead(PC5));
		} else if (pressed & KEY_UP) {
			high = ~high;
			uartDigitalWrite(PD5, high);
			uartDigitalWrite(PD6, high);
			uartDigitalWrite(PD4, high);
			uartDigitalWrite(PD3, high);
			uartDigitalWrite(PC4, high);
			uartDigitalWrite(PC5, high);
		} else if (pressed & KEY_X) {
			break;
		} else if (pressed & KEY_Y) {
			iprintf("\n\n\tatmega software version: 0x%02x", uartSoftwareVersion());
		}
		
		if (uartAvailable()) {
			c = uartGetChar();
			// HACK to filter the 0xffs at ATMEGA startup
			if (c != 0xff)
				iprintf("%c", uartGetChar());
		}
		
		swiWaitForVBlank();
		
	}

	uartClose();
	if (can_reboot())
		reboot();
	return 0;
}
