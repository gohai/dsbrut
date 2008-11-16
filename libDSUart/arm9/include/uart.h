#ifndef _UART_H_
#define _UART_H_


/**
 *	open the UART.
 *
 *	This function opens the IRQ_CARD_LINE interrupt and registers 
 *	the first available timer. irqInit() must be called before uartOpen().
 *	This function blocks until a connection with the cartridge has been 
 *	established. (thus a great place to swap cartridges)
 *	@return				false if already open or no timer interrupt could 
 *						be assigned
 */
bool uartOpen();



/**
 *	receive characters from the device.
 *
 *	@param buffer		buffer
 *	@param size			size of buffer in bytes
 *	@return				number of bytes copied
 */
uint32 uartGet(uint8 *buffer, size_t size);

/**
 *	receive characters from the device and put it in a string.
 *
 *	The resulting string is guaranteed to be NULL-terminated.
 *	@param buffer		buffer
 *	@param size			size of buffer in bytes
 *	@return				number of characters in string (excluding terminating NULL)
 */
uint32 uartGetStr(char *buffer, size_t size);

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
uint32 uartGetLn(char *buffer, size_t size);

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
uint32 uartPut(const uint8 *buffer, size_t size);

/**
 *	write a string to the device.
 *
 *	@param string		NULL-terminated string
 *	@return				number of bytes sent
 */
uint32 uartPutStr(const char *string);

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
uint32 uartAvailable();

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
bool uartSetBaud(uint32 bps);



/******************************
 *                            *
 *  DS Arduino functionality  *
 *                            *
 ******************************/
 
// available pins
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
void uartDigitalWrite(uint8 pin, bool high);

/**
 *	read the value of a pin.
 *
 *	@param pin			pin on ATMEGA168
 *	@return				HIGH (true) or LOW (false)
 */
bool uartDigitalRead(uint8 pin);

/**
 *	writes a PWM value to a pin.
 *
 *	@param pin			pin on ATMEGA168 (works with PD3, PD6, PD5)
 *	@param val			value (0-255)
 */
void uartAnalogWrite(uint8 pin, uint8 val);

/**
 *	read the value of an analog pin.
 *
 *	@param pin			pin on ATMEGA168 (works with PC5, PC4; PC3 on Uart only)
 *	@return				value (0..1023)
 */
uint16 uartAnalogRead(uint8 pin);

/**
 *	send data over the i2c bus.
 *
 *	Use PC4 as SDA and PC5 as SCL line.
 *	The device joins the i2c bus as master. addr has the destination address 
 *	in the lower 7 bits. Internally, they are being left shifted and the read/write 
 *	bit appended, like on Arduino.
 *	@param addr			destination address (slave)
 *	@param data			data to send
 *	@param size			length of data in bytes
 *	@retval 0			success
 *	@retval 1			size to long for (Atmega168) buffer
 *	@retval 2			address send, NACK received
 *	@retval 3			data send, NACK received
 *	@retval	4			other i2c error (lost bus arbitration, bus error, ..)
 *	@retval 255			out of memory
 */
uint8 uartI2CSend(uint8 addr, const uint8 *data, size_t size);

/**
 *	receive data over the i2c bus.
 *
 *	Use PC4 as SDA and PC5 as SCL line.
 *	The device joins the i2c bus as master. addr has the destination address 
 *	in the lower 7 bits. Internally, they are being left shifted and the read/write 
 *	bit appended, like on Arduino.
 *	@param addr			source address (slave)
 *	@param buffer		buffer to fill
 *	@param size			length of buffer in bytes
 *	@return				number of bytes received
 */
uint8 uartI2CReceive(uint8 addr, uint8 *buffer, size_t size);



/**
 *	read the software version from the device.
 *
 *	@return				version number (least significant bit 0..release, 1..devel)
 */
uint8 uartSoftwareVersion();



/**
 *	close the UART device.
 */
void uartClose();


#endif	// _UART_H_
