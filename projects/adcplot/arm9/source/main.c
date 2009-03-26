/*
 *	main.c - plot analog inputs
 *
 *	Copyright Gottfried Haider & Gordan Savicic 2008-2009.
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

#include <nds.h>
#include <stdio.h>
#include <string.h>
// todo: there must be a better way to integrate libdsbrut here
#include "uart.h"
#include "brut.h"


int main(void)
{
	uint32 active = 0;
	uint16 colors[32] = { 0 };
	uint16 i;
	uint16 val[32] = { 0 };
	uint8 x;
	uint8 y;
	
	extern PrintConsole defaultConsole;
	// frame buffer on top
	videoSetMode(MODE_FB0);
	vramSetBankA(VRAM_A_LCD);
	// draw background
	for (i=0; i<256*192; i++) {
		VRAM_A[i] = RGB15(0, 0, 0);
	}
	// console on bottom
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	consoleInit(NULL, defaultConsole.bgLayer, BgType_Text4bpp, BgSize_T_256x256, defaultConsole.mapBase, defaultConsole.gfxBase, false, true);
	
	iprintf("init..");
	iprintf("using timer %u\n\n", uart_init());
	uart_set_spi_rate(1000);
	
	iprintf("a .. enable/disable pc5\n");
	iprintf("b .. enable/disable pc4\n");
	iprintf("x .. enable/disable acclero\n");
	iprintf("y .. enable/disable pc3\n\n");
	
	// set colors
	colors[PC5] = RGB15(31, 0, 0);
	colors[PC4] = RGB15(0, 31, 0);
	colors[ACCEL_X] = RGB15(31, 31, 0);
	colors[ACCEL_Y] = RGB15(0, 31, 31);
	colors[ACCEL_Z] = RGB15(31, 0, 31);
	colors[PC3] = RGB15(0, 0, 31);
	
	// workaround
	scanKeys();
	keysDown();
	
	while(1) {
		
		scanKeys();
		
		if (keysDown() & KEY_A) {
			if (active & 1<<PC5) {
				active &= ~(1<<PC5);
				iprintf("disabled pc5\n");
			} else {
				active |= 1<<PC5;
				iprintf("enabled pc5 (red)\n");
			}
		} else if (keysDown() & KEY_B) {
		if (active & 1<<PC4) {
				active &= ~(1<<PC4);
				iprintf("disabled pc4\n");
			} else {
				active |= 1<<PC4;
				iprintf("enabled pc4 (green)\n");
			}
		} else if (keysDown() & KEY_X) {
			if (active & 1<<ACCEL_X) {
				active &= ~(1<<ACCEL_X);
				active &= ~(1<<ACCEL_Y);
				active &= ~(1<<ACCEL_Z);
				iprintf("disabled accelero\n");
			} else {
				active |= 1<<ACCEL_X;
				active |= 1<<ACCEL_Y;
				active |= 1<<ACCEL_Z;
				iprintf("enabled accelero\n");
			}
		} else if (keysDown() & KEY_Y) {
			if (active & 1<<PC3) {
				active &= ~(1<<PC3);
				iprintf("disabled pc3\n");
			} else {
				active |= 1<<PC3;
				iprintf("enabled pc3 (blue)\n");
			}
		}
		
		// sample data
		for (i=0; i<32; i++) {
			if (active & 1<<i) {
				val[i] = analog_read(i);
			}
		}
		
		// move background
		// todo: there must be a much faster way of doing this
		// memmove didn't work
		for (y=0; y<192; y++) {
			for (x=0; x<255; x++) {
				VRAM_A[y*256+x] = VRAM_A[y*256+x+1];
			}
			if (y == 21) {
				VRAM_A[21*256+255] = RGB15(10, 10, 10);
			} else if (y == 191) {
				VRAM_A[191*256+255] = RGB15(10, 10, 10);
			} else {
				VRAM_A[y*256+255] = RGB15(0, 0, 0);
			}
		}
		// draw
		for (i=0; i<32; i++) {
			if (active & 1<<i) {
				VRAM_A[(191-(val[i]/6))*256+255] = colors[i];
			}
		}
		
		swiWaitForVBlank();
	}
	
	return 0;
}
