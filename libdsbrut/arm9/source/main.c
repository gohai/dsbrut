/*
 *	main.c - libdsbrut Test Application
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
#include "brut.h"
#include "uart.h"


int main(void)
{
	uint32 bps_list[] = { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200 };
	uint8 bps_list_index = 9;
	uint8 old_touch_x = 0;
	bool pins_high = false;
	uint32 spi_rate;
	static char teststring[] = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG";
	touchPosition touch;
	
	consoleDemoInit();
	
	iprintf("libdsbrut test\n");
	iprintf("build %s %s\n\n", __DATE__, __TIME__);
	
	iprintf("init..");
	iprintf("using timer %u\n", uart_init());
	printf("default spi rate.. ~%.2f hz\n\n", uart_get_spi_rate());
	spi_rate = (uint32)uart_get_spi_rate();
	
	//iprintf("a .. send test string\n");
	iprintf("a .. send em4102 on PC5\n");
	iprintf("b .. cycle baud rates\n");
	iprintf("y .. read firmware version\n");
	iprintf("x .. try to reset atmega\n");
	iprintf("\x11 .. digital read on pc5\n");
	iprintf("\x10 .. analog read on pc5\n");
	iprintf("\x1e .. set all pins high/low\n");
	iprintf("\x1f .. read accelerometer\n");
	iprintf("stylo .. do pwm on pc5\n");
	iprintf("left .. decrease spi rate\n");
	iprintf("right .. increase spi rate\n\n");
	
	// workaround
	scanKeys();
	keysDown();
	
	while(1) {
		
		scanKeys();
		
		// workaround for keysHeld() & KEY_TOUCH not working atm
		touchRead(&touch);
		if (touch.px > 0 && touch.px != old_touch_x) {
			iprintf("\npd5 set to %u.. ", touch.px);
			analog_write(PD5, touch.px);
			iprintf("done\n");
			old_touch_x = touch.px;
		}
		
		if (keysDown() & KEY_A) {
			iprintf("\nsending %u bytes.. ", strlen(teststring));
			uart_send(teststring);
			uart_flush();
			iprintf("done\n");
		} else if (keysDown() & KEY_B) {
			bps_list_index++;
			if (bps_list_index == sizeof(bps_list)/sizeof(uint32)) {
				bps_list_index = 0;
			}
			iprintf("\nsetting %lu bps..", bps_list[bps_list_index]);
			uart_set_bps(bps_list[bps_list_index]);
			iprintf("done\n");
		} else if (keysDown() & KEY_X) {
			iprintf("\nresetting (some seconds)..");
			atmega_reset(true);
			iprintf("done\n");
		} else if (keysDown() & KEY_Y) {
			iprintf("\nfirmware version: ");
			iprintf("0x%.2x\n", uart_firmware_ver());
		} else if (keysDown() & KEY_LEFT) {
			iprintf("\npc5 is.. ");
			if (digital_read(PC5)) {
				iprintf("high\n");
			} else {
				iprintf("low\n");
			}
		} else if (keysDown() & KEY_RIGHT) {
			iprintf("\npc5 is.. ");
			iprintf("%u\n", analog_read(PC5));
		} else if (keysDown() & KEY_UP) {
			pins_high = ~pins_high;
			iprintf("\nsetting pins ");
			if (pins_high) {
				iprintf("high.. ");
			} else {
				iprintf("low.. ");
			}
			digital_write(PC5, pins_high);
			digital_write(PC4, pins_high);
			digital_write(PD3, pins_high);
			digital_write(PD4, pins_high);
			digital_write(PD6, pins_high);
			digital_write(PD5, pins_high);
			digital_write(PC3, pins_high);
			iprintf("done\n");
		} else if (keysHeld() & KEY_DOWN) {
			iprintf("\nx %4u y %4u z %4u\n", analog_read(ACCEL_X), analog_read(ACCEL_Y), analog_read(ACCEL_Z));
		} else if (keysHeld() & KEY_L) {
			if (spi_rate > 10) {
				spi_rate -= 10;
				iprintf("\nsetting spi to %lu hz.. ", spi_rate);
				uart_set_spi_rate(spi_rate);
				iprintf("done\n");
			}
		} else if (keysHeld() & KEY_R) {
			spi_rate += 10;
			iprintf("\nsetting spi to %lu hz.. ", spi_rate);
			uart_set_spi_rate(spi_rate);
			iprintf("done\n");
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