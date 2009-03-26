/*
 *	brut.c - DS brut, Arduino-like functions
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
#include <string.h>
#include "brut.h"
#include "uart.h"


uint16 analog_read(uint8 pin)
{
	uint8 msg[] = { '\\', 'a', 0x00, 0x00, 0x00 };
	
	// fix pin mapping
	if (pin == PC5) {
		msg[2] = 5;
	} else if (pin == PC4) {
		msg[2] = 4;
	} else if (pin == PC3) {
		msg[2] = 3;
	}
	
	// we wait for an irq after the 3rd byte (when the adc occurs)
	// this might be a good idea for all arduino-like functions
	uart_write_prio(msg, 5, msg, 0x02);
	uart_wait_prio(2);
	
	return (msg[3]<<8)|msg[4];
}


void analog_write(uint8 pin, uint8 val)
{
	uint8 msg[] = { '\\', 'A', 0x00, 0x00 };
	
	msg[2] = pin;
	msg[3] = val;
	
	uart_write_prio(msg, 4, NULL, 0x00);
	uart_wait_prio(0);
}


uint8 atmega_read_8(uint8 reg)
{
	uint8 msg[] = { '\\', 'r', 0x00, 0x00 };
	
	msg[2] = reg;
	uart_write_prio(msg, 4, msg, 0x00);
	uart_wait_prio(0);
	
	return msg[3];
}


void atmega_reset(bool wait)
{
	// note: untested atm
	
	uint8 msg[] = { '\\', 'x' };
	uint8 ver;
	
	uart_write_prio(msg, 2, NULL, 0x00);
	uart_wait_prio(0);
	
	if (!wait)
		return;
	// wait for the card to be ready again
	do {
		ver = uart_firmware_ver();
		uart_wait();
	} while (ver == 0x00 || ver == 0xff);
}


void atmega_write_8(uint8 reg, uint8 val)
{
	uint8 msg[] = { '\\', 'R', 0x00, 0x00 };
	
	msg[2] = reg;
	msg[3] = val;
	uart_write_prio(msg, 4, NULL, 0x00);
	uart_wait_prio(0);
}


bool digital_read(uint8 pin)
{
	uint8 msg[] = { '\\', 'd', 0x00, 0x00 };
	
	msg[2] = pin;
	
	uart_write_prio(msg, 4, msg, 0x00);
	uart_wait_prio(0);
	
	if (msg[3] != 0x00) {
		return true;
	} else {
		return false;
	}
}


void digital_write(uint8 pin, bool val)
{
	uint8 msg[] = { '\\', 'D', 0x00, 0x00 };
	
	msg[2] = pin;
	if (val) {
		msg[3] = 0x01;
	}
	
	uart_write_prio(msg, 4, NULL, 0x00);
	uart_wait_prio(0);
}


uint8 i2c_receive(uint8 addr, uint8 *dest, uint8 size)
{
	// note: untested atm
	
	uint8 *msg = (uint8*)malloc(5+size);
	uint8 ret;
	
	if (!msg) {
		return 0;		// out of memory
	}
	msg[0] = '\\';
	msg[1] = 'i';
	msg[2] = addr;
	msg[3] = size;
	// all remaining bytes are dummy bytes
	
	// we wait for an irq after the 4th byte (when the transmission 
	// occurs)
	// note: size must be < 32, otherwise we fall back to the timeout
	uart_write_prio(msg, 5+size, msg, 1<<size);
	uart_wait_prio(5);
	
	// actual payload size is returned in the first byte
	if (msg[4] > size) {
		// invalid payload size
		free(msg);
		return 0;		// invalid size returned
	}
	
	// copy to destination buffer
	memcpy(dest, msg+5, msg[4]);
	
	ret = msg[4];
	free(msg);
	return ret;
}


uint8 i2c_send(uint8 addr, const uint8* src, uint8 size)
{
	// note: untested atm
	
	uint8 *msg = (uint8*)malloc(4+size+1);
	uint8 ret;
	
	if (!msg) {
		return 0xff;	// out of memory
	}
	msg[0] = '\\';
	msg[1] = 'I';
	msg[2] = addr;
	msg[3] = size;
	memcpy(msg+4, src, size);
	// final byte is for reading the result
	
	// we wait for an irq on the final byte (when the transmission occurs)
	uart_write_prio(msg, 4+size+1, msg, 0x01);
	uart_wait_prio(5);
	
	ret = msg[4+size];
	free(msg);
	return ret;			// return result
}
