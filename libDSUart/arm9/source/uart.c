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
#include <string.h>
#include "uart.h"
#include "spi.h"

#define UART_FIFO_BLOCK_SIZE 16		// number of bytes that are being moved when the buffer overflows
#define UART_SPI_SPEED CARD_SPI_524_KHZ_CLOCK
#define UART_TIMER_FREQ 100			// internal timer frequency (0.5-32768 Hz)

// internal functions
static void checkInBuffer();
static bool outTraceDone();
static void setOutTrace(uint8_t len, uint8_t *dest);
//static void uartIrqCard();
//static void uartIrqCardLine();
static void uartIrqTimer();
static bool uartPutRaw(const uint8_t *buffer, size_t size);
static void uartUpdate();

char inBuffer[255];
uint8_t inBufferSize = 0;			// number of characters in incoming buffer
uint8_t inBufferHead = 0;			// index of next character to read
char newlineChar = '\n';			// newline character
char outBuffer[255];
uint8_t outBufferSize = 0;
uint8_t outBufferHead = 0;
uint8_t *outTraceDest = NULL;		// set to destination buffer when tracing
short outTraceStart = 0;
short outTraceStop = 0;
uint8_t timer = 0xff;				// timer to use

// for msg 0x04
static const unsigned int uartSpeeds[] = { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200 };


static void checkInBuffer()
{
	// check if we are at the end of the buffer
	if (inBufferSize > 0 && inBufferHead == inBufferSize) {
		inBufferSize = 0;
		inBufferHead = 0;
	}
}


static void setOutTrace(uint8_t len, uint8_t *dest)
{
	// set output tracing
	outTraceDest = dest;
	outTraceStart = outBufferSize-outBufferHead-len+1;
	outTraceStop = outBufferSize-outBufferHead;
}	


static bool outTraceDone()
{
	// check if output tracing is done (bytes received ended up in outTraceDest)
	if (outTraceDest && outTraceStop <= 0) {
		outTraceDest = NULL;
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


//static void uartIrqCardLine()
//{
//	// DEBUG
//	iprintf("received an SPI card line IRQ\n");
//	// we might want to have a callback function here..
//}


static void uartIrqTimer()
{
	// being called while UART is open
	uartUpdate();
}


static void uartUpdate()
{
	uint8_t in;
	
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
	uint8_t i;
	
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
	//irqSet(IRQ_CARD_LINE, uartIrqCardLine);
	//irqEnable(IRQ_CARD_LINE);
	
	// look for available timer
	// TODO: make the timer on demand, 3->0
	for (i=0; i<4; i++) {
		if (TIMER_CR(i) & TIMER_ENABLE)
			continue;
		timer = i;
		irqSet(BIT(i+3), uartIrqTimer);
		irqEnable(BIT(i+3));
		TIMER_DATA(i) = TIMER_FREQ_1024(UART_TIMER_FREQ);
		TIMER_CR(i) = TIMER_ENABLE | TIMER_DIV_1024 | TIMER_IRQ_REQ;
		break;
	}
	if (i == 4) {
		// no timer available
		timer = 0xFF;
		return false;
	}

	config_cardSPI(UART_SPI_SPEED, 1);
	return true;
}


unsigned int uartGet(uint8_t *buffer, size_t size)
{
	unsigned int i;
	
	if (size > inBufferSize-inBufferHead)
		size = inBufferSize-inBufferHead;

	for (i=0; i<size; i++) {
		buffer[i] = inBuffer[inBufferHead];
		inBufferHead++;
	}
	
	checkInBuffer();
	
	return size;
}


unsigned int uartGetStr(char *buffer, size_t size)
{
	unsigned int ret;
	
	ret = uartGet((uint8_t*)buffer, size-1);
	buffer[ret] = '\0';
	
	return ret;
}


unsigned int uartGetLn(char *buffer, size_t size)
{
	unsigned int i;
	
	if (size > inBufferSize-inBufferHead)
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


unsigned int uartPut(const uint8_t *buffer, size_t size)
{
	unsigned int i;
	
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


bool uartPutRaw(const uint8_t *buffer, size_t size)
{
	unsigned int i;
	
	if (size > sizeof(outBuffer)-outBufferSize)
		return false;
	
	for (i=0; i<size; i++) {
		outBuffer[outBufferSize] = buffer[i];
		outBufferSize++;
	}
	
	return true;
}


unsigned int uartPutStr(const char *string)
{
	return uartPut((const uint8_t*)string, strlen(string));
}


bool uartPutChar(char c)
{
	if (outBufferSize+5 < sizeof(outBuffer)) {
		// setup header
		outBuffer[outBufferSize] = 0x05;
		outBuffer[outBufferSize+1] = 0x02;
		outBuffer[outBufferSize+2] = 0x01;
		outBuffer[outBufferSize+3] = 1;
		outBuffer[outBufferSize+4] = (uint8_t)c;
		outBufferSize += 5;
		return true;
	} else {
		return false;
	}
}


unsigned int uartAvailable()
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


bool uartSetBaud(unsigned int bps)
{
	unsigned int i;
	uint8_t msg[] = { 0x05, 0x02, 0x04, 0x00 };
	
	for (i=0; i<sizeof(uartSpeeds); i++) {
		if (bps == uartSpeeds[i]) {
			msg[3] = (uint8_t)(i+1);
			while (!uartPutRaw(msg, 4)) {
				swiIntrWait(0, BIT(timer+3));
			}
			return true;
		}
	}
	return false;
}


void uartDigitalWrite(uint8_t pin, bool high)
{
	uint8_t msg[] = { 0x05, 0x02, 0x00, 0x00 };

	if (high)
		msg[2] = 0x10;
	else
		msg[2] = 0x11;
	msg[3] = pin;
	
	while (!uartPutRaw(msg, 4)) {
		swiIntrWait(0, BIT(timer+3));
	}
}


bool uartDigitalRead(uint8_t pin)
{
	uint8_t msg[] = { 0x05, 0x02, 0x40, pin, 0x02 };
	uint8_t ret;
	
	while(!uartPutRaw(msg, 5)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// read in one byte
	setOutTrace(1, &ret);
	while (!outTraceDone()) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	return (bool)ret;
}


void uartAnalogWrite(uint8_t pin, uint8_t val)
{
	uint8_t msg[] = { 0x05, 0x02, 0x12, pin, val };

	while(!uartPutRaw(msg, 5)) {
		swiIntrWait(0, BIT(timer+3));
	}
}


unsigned short uartAnalogRead(uint8_t pin)
{
	uint8_t msg[] = { 0x05, 0x02, 0x42, pin, 0x00, 0x00 };
	uint8_t ret[2];
	uint8_t tmp;
	
	// fix pin mapping
	if (pin == PC5)
		msg[3] = 5;
	else if (pin == PC4)
		msg[3] = 4;
	else if (pin == PC3)
		msg[3] = 3;
	
	while (!uartPutRaw(msg, 6)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// read in two bytes
	setOutTrace(2, ret);
	while (!outTraceDone()) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// switch byte order
	tmp = ret[0];
	ret[0] = ret[1];
	ret[1] = tmp;
	
	return *((unsigned short*)ret);
}


uint8_t uartSoftwareVersion()
{
	uint8_t msg[] = { 0x05, 0x02, 0x06, 0x00, 0x00 };
	uint8_t ret;
	
	while (!uartPutRaw(msg, 5)) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	// read in a byte
	setOutTrace(1, &ret);
	while (!outTraceDone()) {
		swiIntrWait(0, BIT(timer+3));
	}
	
	return ret;
}


void uartClose()
{
	if (timer != 0xff) {
		TIMER_CR(timer) &= ~TIMER_ENABLE;
		irqDisable(BIT(timer+3));
		irqClear(BIT(timer+3));
	}
	irqDisable(IRQ_CARD_LINE);
	irqClear(IRQ_CARD_LINE);
	irqDisable(IRQ_CARD);
	irqClear(IRQ_CARD);
	
	disable_cardSPI();
	
	timer = 0xff;
}
