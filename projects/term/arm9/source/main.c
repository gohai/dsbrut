/*
 *	main.c - libdsbrut Terminal Application
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
#include <fat.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
// todo: there must be a better way to integrate libdsbrut here
#include "bt.h"
#include "uart.h"


int main(void)
{
	bool echo = false;
	uint32 bps_list[] = { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200 };
	uint8 bps_list_index = 9;
	bool init_fat = false;
	FILE* out = NULL;
	uint32 spi_rate;
	time_t tm;
	
	extern PrintConsole defaultConsole;
	// console on top
	// todo: i really want a fancy background at some point in the future :)
	videoSetMode(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	consoleInit(NULL, defaultConsole.bgLayer, BgType_Text4bpp, BgSize_T_256x256, defaultConsole.mapBase, defaultConsole.gfxBase, true, true);
	// keyboard on bottom
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	keyboardInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x512, 20, 0, false, true);
	keyboardShow();
	
	iprintf("init..");
	iprintf("using timer %u\n", uart_init());
	printf("default spi rate.. ~%.2f hz\n\n", uart_get_spi_rate());
	spi_rate = (uint32)uart_get_spi_rate();
	
	iprintf("a .. enable/disable local echo\n");
	iprintf("b .. cycle baud rates\n");
	iprintf("x .. log input to file\n");
	iprintf("y .. scan for bluetooth devices\n");
	//iprintf("left .. decrease spi rate\n");
	//iprintf("right .. increase spi rate\n");
	iprintf("left .. connect to gps\n");
	iprintf("right .. connect to dummy\n");
	iprintf("up .. disconnect bluetooth\n");
	iprintf("down .. show bluetooth state\n\n");
	
	// workaround
	scanKeys();
	keysDown();
	
	while(1) {
		
		scanKeys();
		
		if (keysDown() & KEY_A) {
			echo = ~echo;
		} else if (keysDown() & KEY_B) {
			bps_list_index++;
			if (bps_list_index == sizeof(bps_list)/sizeof(uint32)) {
				bps_list_index = 0;
			}
			iprintf("\nsetting %u bps..", bps_list[bps_list_index]);
			uart_set_bps(bps_list[bps_list_index]);
			iprintf("done\n");
		} else if (keysDown() & KEY_X) {
			if (!init_fat) {
				if (!fatInitDefault()) {
					iprintf("\ncould not initialize dldi.\n");
				}
				init_fat = true;
			}
			if (out) {
				fclose(out);
				out = NULL;
				iprintf("\nstopped logging.\n");
			} else {
				char tmstr[11] = { 0 };
				char fn[13] = { 0 };
				tm = time(NULL);
				sprintf(tmstr, "%010u", (uint32)tm);
				strncpy(fn, tmstr+2, 8);
				strncpy(fn+8, ".log", 4);
				fn[12] = '\0';
				out = fopen(fn, "w");
				if (out) {
					iprintf("\nstarted logging (%s)..\n", fn);
				} else {
					iprintf("\ncould not log to %s.\n", fn);
				}
			}
		} else if (keysDown() & KEY_Y) {
			iprintf("\nscanning for devices.. ");
			bt_device *list;
			uint8 num = bt_scan(&list, 10);
			uint8 i;
			iprintf("%u found\n", num);
			for (i=0; i<num; i++)
				iprintf("%s %s %s\n", list[i].addr, list[i].name, list[i].cod);
			iprintf("\n");
		// TODO: was keysHeld(), _L
		} else if (keysDown() & KEY_LEFT) {
			iprintf("\nconnecting to gps.. ");
			if (bt_connect("000B0D852342"))
				iprintf("success\n");
			else
				iprintf("error\n");
			/*
			if (spi_rate > 10) {
				spi_rate -= 10;
				iprintf("\nsetting spi to %u hz.. ", spi_rate);
				uart_set_spi_rate(spi_rate);
				iprintf("done\n");
			}
			*/
		// TODO: was keysHeld(), _R
		} else if (keysDown() & KEY_RIGHT) {
			iprintf("\nconnecting to dummy.. ");
			if (bt_connect("001122334455"))
				iprintf("success\n");
			else
				iprintf("error\n");
			/*
			spi_rate += 10;
			iprintf("\nsetting spi to %u hz.. ", spi_rate);
			uart_set_spi_rate(spi_rate);
			iprintf("done\n");
			*/
		} else if (keysDown() & KEY_UP) {
			iprintf("\ndisconnecting bt.. ");
			if (bt_disconnect())
				iprintf("success\n");
			else
				iprintf("error\n");
		} else if (keysDown() & KEY_DOWN) {
			if (bt_connected())
				iprintf("\nbluetooth: connected\n");
			else
				iprintf("\nbluetooth: not connected\n");
		}
		
		// handle keyboard
		int key = keyboardUpdate();
		if (key > 0) {
			if (echo) {
				iprintf("%c", (char)key);
			}
			uart_sendc((char)key);
		}
		
		// handle incoming bytes
		if (uart_available() > 0) {
			uint8 in;
			if (uart_read(&in, 1) == 1) {
				// write into file (if logging)
				if (out) {
					fprintf(out, "%c", in);
				}
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
