#ifndef _UART_H_
#define _UART_H_


/**
 *	initialize uart library.
 *
 *	waits for the device to be ready.
 *	@return			timer irq used (0-3) or 0xff in case of error
 */
uint8 uart_init();


/**
 *	write the content of a buffer to the uart device.
 *
 *	this function internally escapes null-bytes and backspace chars.
 *	@param buf		buffer
 *	@param size		number of bytes to write
 *	@return			number of bytes written
 */
uint16 uart_write(uint8 *buf, uint16 size);


/**
 *	send a string over the uart device.
 *
 *	this function is a helper function that returns only after all 
 *	characters have been put into the output queue.
 *	@param s		null-terminated string to send
 */
void uart_send(char *s);


/**
 *	send a character over the uart device.
 *
 *	this function is a helper function that returns only after the 
 *	character has been put into the output queue.
 *	@param c		character to send
 */
void uart_sendc(char c);


/**
 *	wait until all bytes have been written to the device.
 */
void uart_flush();


/**
 *	return the number of bytes available for reading.
 *
 *	@return			number of bytes available for reading
 */
uint16 uart_available();


/**
 *	read from the uart device.
 *
 *	@param dest		destination buffer
 *	@param size		size of destination buffer
 *	@return			number of bytes copied to destination buffer
 */
uint16 uart_read(uint8 *dest, uint16 size);


/**
 *	read a string from the uart device.
 *
 *	this function is a helper function that produces a null-
 *	terminated string and returns the number of characters 
 *	(excluding the final null).
 *	@param dest		destination string
 *	@param size		size of destination string (in bytes)
 *	@return			characters copied to destination buffer
 */
uint16 uart_readstr(char *dest, uint16 size);


/**
 *	read a line from the uart device.
 *
 *	this function returns all characters up (and including) 
 *	the newline-character. the resulting string is null-
 *	terminated.
 *	@param dest		destination string
 *	@param size		size of destination string (in bytes)
 *	@param nl		newline character (e.g. '\n');
 *	@return			characters copied to destination buffer
 */
uint16 uart_readln(char *dest, uint16 size, char nl);


/**
 *	put the content of a buffer back into the head of the input queue.
 *
 *	@param src		buffer
 *	@param size		number of bytes to copy
 *	@return			true if successful, false if no bytes have been 
 *				copied because queue is full
 */
bool uart_requeue(uint8 *src, uint16 size);


/**
 *	wait for a uart event to occur.
 *
 *	this function returns after a uart timer or spi line irq. it can 
 *	be used for waiting for some input data to become available.
 */
void uart_wait();


/**
 *	set the baudrate to use on the uart side.
 *
 *	115200 bps seems not to work because of baud rate errors (see 
 *	p. 196 of the datasheet), the true baudrate is about 126984 there.
 *	@param bps		baudrate (bytes per second)
 */
void uart_set_bps(uint32 bps);


/**
 *	set the baudrate to use for sending bytes over the spi bus.
 *
 *	this function adjusts the internal timer frequency. the default 
 *	baudrate is being defined by UART_SPI_RATE in uart.c (100Hz at 
 *	the moment).
 *	@param bps		baudrate (bytes per second)
 */
void uart_set_spi_rate(uint32 bps);


/**
 *	set watermarks.
 *
 *	watermarks are used to signal the communication partner to cease 
 *	sending bytes because the user-program does not read the input 
 *	buffer fast enough (or at all) and the input queue is about to 
 *	overflow. the uart-device can use watermarks to do hardware flow-
 *	control. watermarks are disabled by default.
 *	@param high		level for high water (1-100, 0 to disable)
 *	@param low		level for low water (1-100, 0 to disable)
 */
void uart_set_watermarks(uint16 high, uint16 low);


/**
 *	get the effective baudrate used for sending bytes over the spi bus.
 *
 *	@return			baudrate (bytes per second)
 */
float uart_get_spi_rate();


/**
 *	do a priority-write to the uart device.
 *
 *	the content of buf are added to the head of the output queue, 
 *	and are guaranteed to fit in there (as other bytes are being 
 *	descarded to make some room). the content is also not being 
 *	escaped and the response to each byte stored in the dest buffer 
 *	(which can be the same as buf). additionally, irq_bytes holds a 
 *	bitmask for which bytes the library should way for an irq from 
 *	the uart-device before reading in the next byte over spi.
 *	@param buf		buffer
 *	@param size		number of bytes to write
 *	@param dest		destination buffer (can be the same as buf)
 *	@param irq_bytes	bitmask (see above; 0x01 is last byte of buf)
 */
void uart_write_prio(uint8 *buf, uint16 size, uint8 *dest, uint32 irq_bytes);


/**
 *	wait until the priority-write transmission has been completed.
 *
 *	@param timeout		timeout in seconds (or zero to disable)
 *	@return			true if successful, false for timeout
 */
bool uart_wait_prio(uint8 timeout);


/**
 *	get device firmware version.
 *
 *	@return				firmware version
 */
uint8 uart_firmware_ver();


/**
 *	close the device.
 */
void uart_close();


#endif	// _UART_H_
