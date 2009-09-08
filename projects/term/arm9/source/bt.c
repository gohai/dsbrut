/*
 *	bt.c - Bluetooth Support
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
#include <stdio.h>			// only for DEBUG
#include <string.h>
#include "bt.h"
#include "uart.h"


#define BT_KEEP_SIZE 32


static uint8 keep[BT_KEEP_SIZE];
static uint8 keep_size = 0;
static bt_device devices[9];
static uint8 num_devices = 0;


void bt_finish_cmd();
bool bt_prepare_cmd();
void bt_sleep(uint8 sec);
uint8* bt_strstr_raw(uint8 *haystack, uint16 size, uint8 *needle);
bool bt_wait(uint16 min_available, uint8 timeout);


void bt_finish_cmd()
{
	uint16 to_read;
	
	// check if there is anything left in in buffer now
	to_read = uart_available();
	// DEBUG
	if (0 < to_read)
		iprintf("\nbt: %u bytes remaining (%u)\n", to_read, __LINE__);
	
	// send command end string
	uart_send("---\r");
	uart_flush();
	
	// wait for response
	bt_wait(to_read+5, 1);
	// dummy-read everything remaining
	uart_read(NULL, uart_available());
	
	// push back keep buffer
	uart_requeue(keep, keep_size);
	keep_size = 0;
}


bool bt_prepare_cmd()
{
	uint8 *start;
	uint16 to_read;
	
	// drain output
	uart_flush();
	
	// send command string
	bt_sleep(1);
	uart_send("$$$");
	uart_flush();
	bt_sleep(1);
	
	// read latest BT_KEEP bytes
	to_read = uart_available();
	if (BT_KEEP_SIZE < to_read) {
		// dummy read
		uart_read(NULL, to_read-BT_KEEP_SIZE);
	}
	keep_size = uart_read(keep, BT_KEEP_SIZE);
	
	// look for command string
	start = bt_strstr_raw(keep, keep_size, (uint8*)"CMD\r\n");
	if (!start) {
		iprintf("bt: invalid response (%u)\n", __LINE__);
		// push back keep buffer
		uart_requeue(keep, keep_size);
		keep_size = 0;
		return false;
	} else {
		// truncate keep buffer
		keep_size = start-keep;
		return true;
	}
}


void bt_sleep(uint8 sec)
{
	time_t t = time(NULL);
	
	do {
		swiDelay(0);
	} while(time(NULL)-t <= sec);
}


uint8* bt_strstr_raw(uint8 *haystack, uint16 size, uint8 *needle)
{
	uint16 i;
	uint8 match = 0;
	
	for (i=0; i<size; i++) {
		if (haystack[i] == needle[match]) {
			match++;
			if (match == strlen((char*)needle))
				return haystack+i-match+1;
		} else {
			match = 0;
		}
	}
	
	return NULL;
}


bool bt_wait(uint16 min_available, uint8 timeout)
{
	time_t t = time(NULL);
	
	do {
		swiDelay(0);
	} while(uart_available() < min_available && time(NULL)-t <= timeout);
	
	if (min_available <= uart_available())
		return true;
	else
		return false;
}


bool bt_connected()
{
	uint8 ret[3];
	
	if (!bt_prepare_cmd())
		return false;
	
	// send command
	uart_send("GK\r");
	if (!bt_wait(3, 1)) {
		iprintf("\nbt: invalid response (%u)\n", __LINE__);
		bt_finish_cmd();
		return false;
	}
	
	// parse response
	uart_read(ret, sizeof(ret));
	if (ret[0] == '1') {
		bt_finish_cmd();
		return true;
	} else if (ret[0] == '0') {
		bt_finish_cmd();
		return false;
	} else {
		// DEBUG
		iprintf("\nbt: invalid response (%u)\n", __LINE__);
		bt_finish_cmd();
		return false;
	}
}


uint8 bt_scan(bt_device **list, uint8 timeout)
{
	char cmd[6];
	char line[48];
	uint8 i;
	
	// special case for zero timeout (return old list)
	if (timeout == 0) {
		if (list)
			*list = devices;
		return num_devices;
	} else if (48 < timeout) {
		timeout = 48;
	}
	
	if (!bt_prepare_cmd())
		return 0;
	
	num_devices = 0;
	
	// send command
	sprintf(cmd, "I,%u\r", timeout);
	uart_send(cmd);
	while (true) {
		uart_wait();
		if (uart_readln(line, sizeof(line), '\n')) {
			// quit for certain messages
			if (!strncmp(line, "No Devices Found", 16))
				break;
			if (!strncmp(line, "Inquiry Done", 12))
				break;
			// store found devices
			if ('0' <= line[0] && line[0] <= '9') {
				char *start;
				char *end;
				// address
				strncpy(devices[num_devices].addr, line, 12);
				devices[num_devices].addr[12] = '\0';
				// name
				start = line+13;
				end = strchr(start, ',');
				if (!end)
					break;
				// TODO: do bounds check here
				strncpy(devices[num_devices].name, start, end-start);
				devices[num_devices].name[end-start] = '\0';
				// class of device
				// TODO: do bounds check here
				strncpy(devices[num_devices].cod, end+1, strlen(end+1)-2);
				devices[num_devices].cod[strlen(end+1)-2] = '\0';
				num_devices++;
			}
		}
	}
	
	bt_finish_cmd();
	
	// clean up list of devices
	for (i=num_devices; i<9; i++) {
		devices[i].addr[0] = '\0';
		devices[i].name[0] = '\0';
		devices[i].cod[0] = '\0';
	}
	
	// return pointer to our list of devices
	if (list)
		*list = devices;
	
	return num_devices;
}


bool bt_set_name(const char *name)
{
	char cmd[21];
	uint8 ret[5];
	
	if (16 < strlen(name))
		return false;
	if (!bt_prepare_cmd())
		return false;
	
	// send command
	sprintf(cmd, "SN,%s\r", name);
	uart_send(cmd);
	if (!bt_wait(5, 1)) {
		bt_finish_cmd();
		return false;
	}
	
	// parse response
	uart_read(ret, sizeof(ret));
	if (!strncmp((char*)ret, "AOK", 3)) {
		bt_finish_cmd();
		return true;
	} else {
		bt_finish_cmd();
		return false;
	}
}
