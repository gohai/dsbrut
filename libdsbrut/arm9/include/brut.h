#ifndef _BRUT_H_
#define _BRUT_H_


// available pins, left to right
#define PC5			19
#define PC4			18
#define PD3			3
#define PD4			4
#define PD6			6
#define PD5			5
#define PC3			17				// only exposed on ds brut uart only

// for the adxl330 3-axis accelerometer
#define ACCEL_X			1
#define ACCEL_Y			2
#define ACCEL_Z			3

// to helo you arduino people
#define HIGH			true
#define LOW			false


/**
 *	read the value of an analog pin.
 *
 *	works with PC5, PC4; PC3 (ds brut uart only)
 *	you can use this function to read from a build-in adxl330 
 *	accelerometer, using ACCEL_{X,Y,Z} as pin.
 *	@param pin		pin to use (see pin definitions above)
 *	@return			value (0-1023)
 */
uint16 analog_read(uint8 pin);


/**
 *	start a fast adc conversion of an analog pin.
 *
 *	this function continuously returns the value of the analog pin as an 
 *	uint8 (8 bit) in the normal input queue (so read with uart_read(), 
 *	etc). until this function is called again with start set to false, 
 *	all other data send to the device will be ignored.
 *	@param start		true to start, false to stop
 *	@param pin		pin to use (see pin definitions above)
 */
void analog_read_fast(bool start, uint8 pin);

 
/**
 *	write a pwm value to a pin.
 *
 *	works with PD3, PD6, PD5
 *	@param pin		pin to use (see pin definitions above)
 *	@param val		value
 */
void analog_write(uint8 pin, uint8 val);


/**
 *	reset the device.
 *
 *	this function triggers a watchdog reset on the atmega micro-
 *	controller. a modified bootloader is necessary for this to work, 
 *	otherwise no reset will be performed.
 *	@param wait		wait for the card to become available again
 */
void atmega_reset(bool wait);


/**
 *	read registers on the device.
 *
 *	see atmega168 datasheet, p. 343f.
 *	@param reg		register address
 *	@return			register value
 */
uint8 atmega_read_8(uint8 reg);


/**
 *	write to registers on the device.
 *
 *	see atmega168 datasheet, p. 343f.
 *	@param reg		register address
 *	@param val		value to set
 */
void atmega_write_8(uint8 reg, uint8 val);


/**
 *	read the value of a pin.
 *
 *	@param pin		pin to use (see pin definitions above)
 *	@return			HIGH or LOW
 */
bool digital_read(uint8 pin);


/**
 *	set a pin either high or low.
 *
 *	@param pin		pin to use (see pin definitions above)
 *	@param val		HIGH or LOW
 */
void digital_write(uint8 pin, bool val);


/**
 *	receive data over the i2c bus.
 *
 *	use PC4 as SDA and PC5 as SCL line.
 *	the device joins the i2c bus as master. addr has the destination 
 *	address in the lower 7 bits. internally, they are being left-shifted 
 *	and the read/write bit appended (like on arduino).
 *	@param addr		source address (slave)
 *	@param dest		destination buffer
 *	@param size		size of destination buffer in bytes
 *	@return			number of bytes received
 */
uint8 i2c_receive(uint8 addr, uint8 *dest, uint8 size);


/**
 *	send data over the i2c bus.
 *
 *	use PC4 as SDA and PC5 as SCL line.
 *	the device joins the i2c bus as master. addr has the destination 
 *	address in the lower 7 bits. internally, they are being left-shifted 
 *	and the read/write bit appended (like on arduino).
 *	@param addr		destination address (slave)
 *	@param src		data to send
 *	@param size		number of bytes to send
 *	@retval 0		success
 *	@retval 1		size to big for (atmega) buffer
 *	@retval 2		address send, nack received
 *	@retval 3		data send, nack received
 *	@retval 4		other i2c error (lost bus arbitration, bus error)
 *	@retval 255		out of memory
 */
uint8 i2c_send(uint8 addr, const uint8 *src, uint8 size);


/**
 *	send an EM4102 sequence
 *
 *	data is expected to be five bytes. first byte holds the manufacturer 
 *	id while the other ones hold the unique (card) id.
 *	needs firmware version 0x13 or higher.
 *	@param pin		pin to use (see pin definitions above)
 *	@param data		data buffer (expected to be five bytes)
 *	@param tries	number of consecutive times to send the sequence
 *	@return			true if successful, false if not
 */
bool em4102_send(uint8 pin, const uint8* data, uint8 tries);


#endif	// _BRUT_H_
