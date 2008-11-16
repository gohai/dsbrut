/**
 *	Code for controlling the ADJD-S371 ("Color Light Sensor", available from 
 *	SparkFun)
 */


/**
 *	set the capacitor settings of the sensor.
 *
 *	Valid values range from 0 - 15. Higher values will result in lower 
 *	sensor output.
 *	@param red			red capacitance value (0-15)
 *	@param green		green capacitance value (0-15)
 *	@param blue			blue capacitance value (0-15)
 *	@param clear		clear channal capacitance value (0-15)
 *	@return				true if successful, false if not
 */
bool adjdSetCaps(uint8 red, uint8 green, uint8 blue, uint8 clear);

/**
 *	set the integration time slot settings of the sensor.
 *
 *	Valid values range from 0 - 4095. Higher values will result in higher 
 *	sensor output.
 *	@param red			red integration time value (0-4095)
 *	@param green		green integration time value (0-4095)
 *	@param blue			blue integration time value (0-4095)
 *	@param clear		clear channal integration time value (0-4095)
 *	@return				true if successful, false if not
 */
bool adjdSetInt(uint16 red, uint16 green, uint16 blue, uint16 clear);

/**
 *	get a color reading.
 *
 *	The 10-bit result is being written to the addresses given. NULL skips the 
 *	parameter.
 *	@param red			pointer to a variable being filled with the red ADC reading
 *	@param green		pointer to a variable being filled with the green ADC reading
 *	@param blue			pointer to a variable being filled with the blue ADC readling
 *	@param clear		pointer to a variable being filled with the clear channel ADC readling
 *	@return 			true if successful, false if not (variables have not been changed)
 */
bool adjdGetColors(uint16 *red, uint16 *green, uint16 *blue, uint16 *clear);

/**
 *	set the DS brut pin used for controlling the module's LED.
 *
 *	@param pin			pin to use (zero disables turning the LED on and off for 
 *						data aquisition)
 */
void adjdSetLedPin(uint8 pin);


/**
 *	automatically adjust capacitor settings.
 *
 *	Place the sensor in front of a white (or max-brightness) surface 
 *	and call this function. Make sure to set the time slot settings to 
 *	a medium value before.
 *	@return				capacitor setting now in place
 */
uint8 adjdAutoCaps();


/**
 *	automatically adjust integration time settings.
 *
 *	Place the sensor in front of a white (or max-brightness) surface 
 *	and call this function. This may take a while..
 *	@return				average channel level
 */
uint16 adjdAutoInt();


/**
 *	return a RGB15 color for display on the NDS.
 *
 * @param red			red color value
 * @param green			green color value
 * @param blue			blue color value
 * @param level			expected maximum color level
 * @return				RGB15 color
 */
uint16 adjdColorToRGB15(uint16 red, uint16 green, uint16 blue, uint16 level);
