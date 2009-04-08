/*
 *	NDS UART Library
 *
 *	Copyright Gottfried Haider & Gordon Savicic 2008.
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
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "spi.h"

#define UART_FIFO_BLOCK_SIZE 16		// number of bytes that are being moved when the buffer overflows
#define UART_SPI_SPEED CARD_SPI_1_MHZ_CLOCK
#define UART_TIMER_FREQ 20000			// WRD was 100, internal timer frequency (0.5-32768 Hz)

// internal functions
static void checkInBuffer();
static bool outTraceDone();
static void setOutTrace(uint8 len, uint8 *dest, bool waitForIrq);
//static void uartIrqCard();
static void uartIrqCardLine();
static void uartIrqTimer();
//static bool uartPutRaw(const uint8 *buffer, size_t size);
static void uartUpdate();

uint8 inBuffer[390];
uint16 inBufferSize = 0;			// number of characters in incoming buffer
uint16 inBufferHead = 0;			// index of next character to read
char newlineChar = '\n';		// newline character
uint8 outBuffer[500];
uint16 outBufferSize = 0;
uint16 outBufferHead = 0;
uint8 *outTraceDest = NULL;		// set to destination buffer when tracing
bool outTraceIrq = false;		// wait for Card Line interrupt if true
int16 outTraceStart = 0;
int16 outTraceStop = 0;
uint8 timer = 0xff;				// timer to use
uint8 dat_array[264];

// for msg 0x04
static const uint32 uartSpeeds[] = { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200 };


static void checkInBuffer()
{
	// check if we are at the end of the buffer
	if (inBufferSize > 0 && inBufferHead == inBufferSize) {
		inBufferSize = 0;
		inBufferHead = 0;
	}
}


static void setOutTrace(uint8 len, uint8 *dest, bool waitForIrq)
{
	// set output tracing
	outTraceDest = dest;
	outTraceIrq = waitForIrq;
	outTraceStart = outBufferSize-outBufferHead-len+1;
	outTraceStop = outBufferSize-outBufferHead;
}	


static bool outTraceDone()
{
	// check if output tracing is done (bytes received ended up in outTraceDest)
	if (outTraceDest && outTraceStop <= 0) {
		outTraceDest = NULL;
		outTraceIrq = false;
		return true;
	} else {
		return false;
	}
}


//static void uartIrqCard()
//{
//	// DEBUG
//	iprintf("received an SPI card IRQ\n");
//}


static void uartIrqCardLine()
{
//	// DEBUG
//	iprintf("received an SPI card line IRQ\n");
//	// we might want to have a callback function here..

	// we re-enable the timer interrupt here
	// TODO: not sure if we need to sleep here (swiDelay())
	if (timer != 0xff) {
		irqEnable(BIT(timer+3));
//		TIMER_CR(timer) = TIMER_ENABLE | TIMER_DIV_1024 | TIMER_IRQ_REQ;
		TIMER_CR(timer) = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;
	}
}


static void uartIrqTimer()
{
	// being called while UART is open
	uartUpdate();
}


static void uartUpdate()
{
	uint8 in;
	
	if (outBufferHead < outBufferSize) {
		writeBlocking_cardSPI(outBuffer[outBufferHead]);
		outBufferHead++;
		
		// check if we are at the end of the buffer
		if (outBufferHead == outBufferSize) {
			outBufferSize = 0;
			outBufferHead = 0;
		}
	} else {
		// write dummy byte
		writeBlocking_cardSPI(0x00);
	}
	
	readBlocking_cardSPI(&in);
	
	// output tracing
	if (outTraceDest) {
		outTraceStart--;
		outTraceStop--;
		if (outTraceStart <= 0 && 0 <= outTraceStop) {
			outTraceDest[0-outTraceStart] = in;
		}
		// stop timer here when we are tracing and want to wait for an Card Line interrupt (EXPERIMENTAL)
		if (outTraceIrq && outTraceStart == 1) {
			TIMER_CR(timer) &= ~TIMER_ENABLE;
			irqDisable(BIT(timer+3));
		}
		// TODO: not sure if we don't eat to many characters here
		return;
	}
	
	// TODO: we might want be able to read in NULL bytes in the future
	if (in == 0x00)
		return;
		
	// FIFO
	if (inBufferSize >= sizeof(inBuffer)) {
		memmove(inBuffer, inBuffer+UART_FIFO_BLOCK_SIZE, sizeof(inBuffer)-UART_FIFO_BLOCK_SIZE);
		inBufferSize = sizeof(inBuffer)-UART_FIFO_BLOCK_SIZE;
	}
		
	inBuffer[inBufferSize] = in;
	inBufferSize++;
}


bool uartOpen()
{
	uint8 i;
	
	// check if uart is already open
	if (timer != 0xff)
		return false;
	
	// setup access
#ifdef ARM9
	REG_EXMEMCNT &= ~ARM7_OWNS_CARD;
#else
	REG_EXMEMCNT |= ARM7_OWNS_CARD;
#endif	// ARM9

	init_cardSPI();

	// setup interrupts
	//irqSet(IRQ_CARD, uartIrqCard);
	//irqEnable(IRQ_CARD);
	irqSet(IRQ_CARD_LINE, uartIrqCardLine);
	irqEnable(IRQ_CARD_LINE);
	
	// look for available timer
	// TODO: make the timer on demand, 3->0
	for (i=0; i<4; i++) {
		if (TIMER_CR(i) & TIMER_ENABLE)
			continue;
		timer = i;
		irqSet(IRQ_MASK BIT(i+3), uartIrqTimer);
		irqEnable(BIT(i+3));
//		TIMER_DATA(i) = TIMER_FREQ_1024(UART_TIMER_FREQ);
		TIMER_DATA(i) = TIMER_FREQ(UART_TIMER_FREQ);
//		TIMER_CR(i) = TIMER_ENABLE | TIMER_DIV_1024 | TIMER_IRQ_REQ;
		TIMER_CR(i) = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;
		break;
	}
	if (i == 4) {
		// no timer available
		timer = 0xFF;
		return false;
	}

	config_cardSPI(UART_SPI_SPEED, 1);
	
	// wait for card to become ready
	do {
		i = uartSoftwareVersion();
	} while (i == 0x00 || i == 0xff);
	
	return true;
}


uint32 uartGet(uint8 *buffer, size_t size)
{
	uint32 i;
	
	if (size > (inBufferSize-inBufferHead))
		size = (inBufferSize-inBufferHead);

	for (i=0; i<size; i++) {
		buffer[i] = inBuffer[inBufferHead];
		inBufferHead++;
	}
	
	checkInBuffer();
	
	return size;
}


uint32 uartGetStr(char *buffer, size_t size)
{
	uint32 ret;
	
	ret = uartGet((uint8*)buffer, size-1);
	buffer[ret] = '\0';
	
	return ret;
}


uint32 uartGetLn(char *buffer, size_t size)
{
	uint32 i;
	
	if (size > (inBufferSize-inBufferHead))
		size = inBufferSize-inBufferHead;

	// look for newline character
	for (i=0; i<size-1; i++) {
		if (inBuffer[inBufferHead+i] == newlineChar)
			break;
	}
	if (i == size-1)
		return 0;		// newline character not found
	else
		size = i;
		
	for (i=0; i<size; i++) {
		buffer[i] = inBuffer[inBufferHead];
		inBufferHead++;
	}
	// make string NULL-terminated
	buffer[i] = '\0';
	
	checkInBuffer();
	
	return size;
}


char uartGetChar()
{
	char ret = 0x00;
	
	if (inBufferHead < inBufferSize) {
		ret = inBuffer[inBufferHead];
		
		inBufferHead++;
		checkInBuffer();
	}
	
	return ret;
}


uint32 uartPut(const uint8 *buffer, size_t size)
{
	uint32 i;
	
	if (size > sizeof(outBuffer)-outBufferSize-4)
		size = sizeof(outBuffer)-outBufferSize-4;
	
	// setup header
	outBuffer[outBufferSize] = 0x05;
	outBuffer[outBufferSize+1] = 0x02;
	outBuffer[outBufferSize+2] = 0x01;
	outBuffer[outBufferSize+3] = size;
	outBufferSize += 4;
	for (i=0; i<size; i++) {
		outBuffer[outBufferSize] = buffer[i];
		outBufferSize++;
	}
	
	return size;
}


bool uartPutRaw(const uint8 *buffer, size_t size)
{
	uint32 i;
	
	if (size > sizeof(outBuffer)-outBufferSize)
		return false;
	
	for (i=0; i<size; i++) {
		outBuffer[outBufferSize] = buffer[i];
		outBufferSize++;
	}
	
	return true;
}


uint32 uartPutStr(const char *string)
{
	return uartPut((const uint8*)string, strlen(string));
}


bool uartPutChar(char c)
{
	if (outBufferSize+5 < sizeof(outBuffer)) {
		// setup header
		outBuffer[outBufferSize] = 0x05;
		outBuffer[outBufferSize+1] = 0x02;
		outBuffer[outBufferSize+2] = 0x01;
		outBuffer[outBufferSize+3] = 1;
		outBuffer[outBufferSize+4] = (uint8)c;
		outBufferSize += 5;
		return true;
	} else {
		return false;
	}
}


uint32 uartAvailable()
{
	return inBufferSize-inBufferHead;
}


void uartClearIn()
{
	inBufferSize = 0;
	inBufferHead = 0;
}


void uartClearOut()
{
	// TODO: test
	outBufferSize = 0;
	outBufferHead = 0;
}


void uartSetNewlineChar(char c)
{
	newlineChar = c;
}


bool uartSetBaud(uint32 bps)
{
	uint32 i;
	uint8 msg[] = { 0x05, 0x02, 0x04, 0x00 };
	
	for (i=0; i<sizeof(uartSpeeds); i++) {
		if (bps == uartSpeeds[i]) {
			msg[3] = (uint8)(i+1);
			while (!uartPutRaw(msg, 4)) {
				swiIntrWait(0, BIT(timer+3));
			}
			return true;
		}
	}
	return false;
}


void uartDigitalWrite(uint8 pin, bool high)
{
	uint8 msg[] = { 0x05, 0x02, 0x00, 0x00 };

	if (high)
		msg[2] = 0x10;
	else
		msg[2] = 0x11;
	msg[3] = pin;
	
	while (!uartPutRaw(msg, 4)) {
		swiIntrWait(0, BIT(timer+3));
	}
}


bool uartDigitalRead(uint8 pin)
{
	uint8 msg[] = { 0x05, 0x02, 0x40, pin, 0x02 };
	uint8 ret;
	
	while(!uartPutRaw(msg, 5)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// read in one byte
	setOutTrace(1, &ret, false);
	while (!outTraceDone()) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	return (bool)ret;
}


void uartAnalogWrite(uint8 pin, uint8 val)
{
	uint8 msg[] = { 0x05, 0x02, 0x12, pin, val };

	while(!uartPutRaw(msg, 5)) {
		swiIntrWait(0, BIT(timer+3));
	}
}


uint16 uartAnalogRead20kHz(uint8 pin, uint8 *array)
{
	uint8 msg[] = { 0x05, 0x02, 0x42, pin, 0x00, 0x00 };
	
	while (!uartPutRaw(msg, 6)) {
		swiIntrWait(0, BIT(timer+3));
	}
	uartGet(array, 384);

	return 1;
}


uint8 uartI2CSend(uint8 addr, const uint8 *data, size_t size)
{
	uint8 *msg = (uint8*)malloc(5+size+1);
	uint8 ret;
	
	if (!msg)
		return 0xff;	// error: out of memory
	msg[0] = 0x05;
	msg[1] = 0x02;
	msg[2] = 0x44;
	msg[3] = addr;		// destination address
	msg[4] = size;		// payload size
	memcpy(&msg[5], data, size);
	// final byte is dummy byte
	
	while (!uartPutRaw(msg, 5+size+1)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// read in one byte
	setOutTrace(1, &ret, true);
	while (!outTraceDone()) {
		swiIntrWait(0, BIT(timer+3)|IRQ_CARD_LINE);
	}
	
	free(msg);
	return ret;
}


uint8 uartI2CReceive(uint8 addr, uint8 *buffer, size_t size)
{
	uint8 *msg = (uint8*)malloc(5+size+1);
	uint8 *ret = (uint8*)malloc(size+1);
	
	if (!msg || !ret) {
		free(msg);
		free(ret);
		return 0;		// error: out of memory
	}
	msg[0] = 0x05;
	msg[1] = 0x02;
	msg[2] = 0x46;
	msg[3] = addr;
	msg[4] = size;
	// all other ones are dummy bytes
	
	while (!uartPutRaw(msg, 5+size+1)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// read in response
	setOutTrace(size+1, ret, true);
	while (!outTraceDone()) {
		swiIntrWait(0, BIT(timer+3)|IRQ_CARD_LINE);
	}
	
	// actual payload size is returned in the first byte
	if (ret[0] > size) {
		free(msg);
		free(ret);
		return 0;		// error: invalid size returned
	}
	
	// copy payload to buffer
	memcpy(buffer, &ret[1], ret[0]);
	size = ret[0];
	
	free(msg);
	free(ret);
	return size;		// return actual payload size
}


uint8 uartSoftwareVersion()
{
	uint8 msg[] = { 0x05, 0x02, 0x06, 0x00, 0x00 };
	uint8 ret;
	
	while (!uartPutRaw(msg, 5)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// read in a byte
	setOutTrace(1, &ret, false);
	while (!outTraceDone()) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	return ret;
}


void uartHardwareReset(bool wait)
{
	uint8 msg[] = { 0x05, 0x02, 0x08 };
	uint8 i;
	
	while (!uartPutRaw(msg, 3)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	if (!wait)
		return;
		
	// wait for the card to become ready again
	do {
		i = uartSoftwareVersion();
	} while (i == 0x00 || i == 0xff);
}


void uartClose()
{
	if (timer != 0xff) {
		TIMER_CR(timer) &= ~TIMER_ENABLE;
		irqDisable(BIT(timer+3));
		irqClear(IRQ_MASK BIT(timer+3));
	}
	irqDisable(IRQ_CARD_LINE);
	irqClear(IRQ_CARD_LINE);
	irqDisable(IRQ_CARD);
	irqClear(IRQ_CARD);
	
	disable_cardSPI();
	
	timer = 0xff;
}
