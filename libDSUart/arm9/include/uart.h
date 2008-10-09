#ifndef _UART_H_
#define _UART_H_


/**
 *	open the UART.
 *
 *	This function opens the IRQ_CARD_LINE interrupt and registers 
 *	the first available timer. irqInit() must be called before uartOpen().
 */
bool uartOpen();



/**
 *	receive characters from the device.
 *
 *	@param buffer		buffer
 *	@param size			size of buffer in bytes
 *	@return				number of bytes copied
 */
unsigned int uartGet(uint8_t *buffer, size_t size);

/**
 *	receive characters from the device and put it in a string.
 *
 *	The resulting string is guaranteed to be NULL-terminated.
 *	@param buffer		buffer
 *	@param size			size of buffer in bytes
 *	@return				number of characters in string (excluding terminating NULL)
 */
unsigned int uartGetStr(char *buffer, size_t size);

/**
 *	receive a line from the device and put it in a string.
 *
 *	This function copies characters up to the first newline character 
 *	to a buffer. The resulting string is guaranteed to be NULL-terminated 
 *	and includes the newline character (default: \n).
 *	@param buffer		buffer
 *	@param size			size of buffer in bytes
 *	@return				number of characters in string (excluding terminating NULL)
 */
unsigned int uartGetLn(char *buffer, size_t size);

/**
 *	read a character from the device.
 *
 *	@return				character (default: 0x00)
 */
char uartGetChar();



/**
 *	write characters to the device.
 *
 *	Over SPI, the string is sent with 0x05, 0x02, 0x01 (message type) and 
 *	length as header.
 *	@param buffer		buffer
 *	@param size			number of bytes to send
 *	@return				number of bytes sent
 */
unsigned int uartPut(const uint8_t *buffer, size_t size);

/**
 *	write a string to the device.
 *
 *	@param string		NULL-terminated string
 *	@return				number of bytes sent
 */
unsigned int uartPutStr(const char *string);

/**
 *	write a character to the device.
 *
 *	@param c			character
 *	@return				true if successful, false if not
 */
bool uartPutChar(char c);



/**
 *	return the number of bytes available for reading.
 *
 *	@return				number of available bytes
 */
unsigned int uartAvailable();

/**
 *	clear the input buffer.
 */
void uartClearIn();

/**
 *	clear the output buffer.
 */
void uartClearOut();

/**
 *	set the newline character used for uartGetLn().
 *
 *	@param c			newline character (default: \n)
 */
void uartSetNewlineChar(char c);



/**
 *	set the baudrate.
 *
 *	Available baud rates: 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 
 *	38400, 57600, 115200 - however, 300, 1200 and 115200 don't seem to work 
 *	reliable atm.
 *	@param bps			baud rate (default: 57600)
 *	@return				true if successful, false if not (unsupported baud rate or output 
 *						buffer full)
 */
bool uartSetBaud(unsigned int bps);



/******************************
 *                            *
 *  DS Arduino functionality  *
 *                            *
 ******************************/
 
// available pins (on DS GIG Master at least :)
// left to right
#define PC5 19
#define PC4 18
#define PD3 3
#define PD4 4
#define PD6 6
#define PD5 5
// only available on Uart only cartridge
#define PC3 17

// to help you Arduino people
#define HIGH 1
#define LOW 0

/**
 *	set a pin either high or low.
 *
 *	@param pin			pin on ATMEGA168
 *	@param high			HIGH or LOW
 */
void uartDigitalWrite(uint8_t pin, bool high);

/**
 *	read the value of a pin.
 *
 *	@param pin			pin on ATMEGA168
 *	@return				HIGH (true) or LOW (false)
 */
bool uartDigitalRead(uint8_t pin);

/**
 *	writes a PWM value to a pin.
 *
 *	@param pin			pin on ATMEGA168 (works with PD3, PD6, PD5)
 *	@param val			value (0-255)
 */
void uartAnalogWrite(uint8_t pin, uint8_t val);

/**
 *	read the value of an analog pin.
 *
 *	@param pin			pin on ATMEGA168 (works with PC5, PC4; PC3 on Uart only)
 *	@return				value (0..1023)
 */
unsigned short uartAnalogRead(uint8_t pin);



/**
 *	read the software version from the device.
 *
 *	@return				version number (least significant bit 0..release, 1..devel)
 */
uint8_t uartSoftwareVersion();



/**
 *	close the UART device.
 */
void uartClose();


#endif	// _UART_H_
