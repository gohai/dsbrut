/*
 *	Code for controlling the ADJD-S371 ("Color Light Sensor", available from 
 *	SparkFun)
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
#include "adjd-s371.h"
#include "uart.h"

#define ADJD_ADDR 0x74

uint8 adjdLightPin = 0;


bool adjdSetCaps(uint8 red, uint8 green, uint8 blue, uint8 clear)
{
	uint8 out[2];
	
	if (15 < red || 15 < green || 15 < blue || 15 < clear)
		return false;		// error: out of range
		
	out[0] = 0x06;			// CAP_RED
	out[1] = red;
	if (uartI2CSend(ADJD_ADDR, out, 2))
		return false;		// error: i2c error
	out[0] = 0x07;			// CAP_GREEN
	out[1] = green;
	if (uartI2CSend(ADJD_ADDR, out, 2))
		return false;
	out[0] = 0x08;			// CAP_BLUE
	out[1] = blue;
	if (uartI2CSend(ADJD_ADDR, out, 2))
		return false;
	out[0] = 0x09;			// CAP_CLEAR
	out[1] = clear;
	if (uartI2CSend(ADJD_ADDR, out, 2))
		return false;
		
	return true;
}


bool adjdSetInt(uint16 red, uint16 green, uint16 blue, uint16 clear)
{
	uint16 val;
	uint8 out[2];
	uint8 i;
	
	for (i=0; i<4; i++) {
		if (i == 0) {
			val = red;
		} else if (i == 1) {
			val = green;
		} else if (i == 2) {
			val = blue;
		} else if (i == 3) {
			val = clear;
		}
		
		if (val > 4095)
			return false;	// error: out of range
			
		out[0] = 0x0a+i*2;
		out[1] = (uint8)(val & 0xFF);
		if (uartI2CSend(ADJD_ADDR, out, 2))
			return false;	// error: i2c error
		out[0] = 0x0b+i*2;
		out[1] = (uint8)(val>>8);
		if (uartI2CSend(ADJD_ADDR, out, 2))
			return false;
	}
	
	return true;
}


bool adjdGetColors(uint16 *red, uint16 *green, uint16 *blue, uint16 *clear)
{
	uint16 val;
	uint8 out[2];
	uint8	in = 0xff;
	uint8 i;
	
	// turn on the LED
	if (adjdLightPin)
		uartDigitalWrite(adjdLightPin, true);
		
	// TODO: delay?
	
	// get reading
	out[0] = 0x00;			// CTRL
	out[1] = 0x01;			// GSSR = true
	if (uartI2CSend(ADJD_ADDR, out, 2))
		goto out_error;		// error: i2c error
	
	// wait for reading to complete
	while (in != 0x00) {
		if (uartI2CSend(ADJD_ADDR, out, 1) || uartI2CReceive(ADJD_ADDR, &in, 1) != 1)
			goto out_error;
	}
	
	// read in values
	for (i=0; i<4; i++) {
		out[0] = 0x40+i*2;
		if (uartI2CSend(ADJD_ADDR, out, 1) || uartI2CReceive(ADJD_ADDR, &in, 1) != 1)
			goto out_error;
		val = in;
		out[0] = 0x41+i*2;
		if (uartI2CSend(ADJD_ADDR, out, 1) || uartI2CReceive(ADJD_ADDR, &in, 1) != 1)
			goto out_error;
		val |= (in<<8);
		
		if (i == 0 && red) {
			*red = val;
		} else if (i == 1 && green) {
			*green = val;
		} else if (i == 2 && blue) {
			*blue = val;
		} else if (i == 3 && clear) {
			*clear = val;
		}
	}
	
	// turn the LED off
	if (adjdLightPin)
		uartDigitalWrite(adjdLightPin, false);
	
	return true;
	
out_error:
	// turn the LED off
	if (adjdLightPin)
		uartDigitalWrite(adjdLightPin, false);
	
	return false;
}


void adjdSetLedPin(uint8 pin)
{
	adjdLightPin = pin;
}


uint8 adjdAutoCaps()
{
	int i;
	uint16 colors[4];
	uint16 err;
	uint16 minErr = 4*1000+1;
	uint8 minErrCaps = 0;
	
	for (i=15; i>=0; i--) {
		adjdSetCaps(i, i, i, i);
		if (!adjdGetColors(&colors[0], &colors[1], &colors[2], &colors[3]))
			continue;
		err = abs(1000-colors[0])+abs(1000-colors[1])+abs(1000-colors[2])+abs(1000-colors[3]);
		
		if (err < minErr) {
			minErr = err;
			minErrCaps = i;
		}
	}
	
	adjdSetCaps(minErrCaps, minErrCaps, minErrCaps, minErrCaps);
		
	return minErrCaps;
}


uint16 adjdAutoInt()
{
	uint16 colors[4];
	uint16 integr[4] = { 2047, 2047, 2047, 2047 };		// integration time settings
	uint8 maxChan = 0;									// channel with max level
	uint16 sum[3] = { 0, 0, 0 };						// sum of multiple samples
	uint16 maxChanVal = 0;								// value of this channel
	float avg[3];										// moving avg of channel levels
	uint16 err = 0;										// difference between avgs and maxChanVal
	uint16 step;										// amount to add/substract to integrs
	int i;
	
	
	// set default integration time value
	if (!adjdSetInt(integr[0], integr[1], integr[2], integr[3]))
		return 0;
	
	// get strongest channel
	for (i=0; i<5; i++) {
		if (!adjdGetColors(&colors[0], &colors[1], &colors[2], &colors[3]))
			continue;
		sum[0] += colors[0];
		sum[1] += colors[1];
		sum[2] += colors[2];
	}
	
	if (sum[0] >= sum[1] && sum[0] >= sum[2]) {
		maxChan = 0;
		maxChanVal = sum[0] / 5;
	} else if (sum[1] >= sum[0] && sum[1] >= sum[2]) {
		maxChan = 1;
		maxChanVal = sum[1] / 5;
	} else if (sum[2] >= sum[0] && sum[2] >= sum[1]) {
		maxChan = 2;
		maxChanVal = sum[2] / 5;
	}
	
	// setup starting average
	for (i=0; i<3; i++) {
		avg[i] = sum[i] / 5.0;
	}
	
	
	// iterate until we have a low error or one of the integr values is maxed out
	while (true) {
		
		// get color reading
		if (!adjdGetColors(&colors[0], &colors[1], &colors[2], &colors[3]))
			continue;
		
		// calculate average
		for (i=0; i<3; i++) {
			avg[i] = avg[i]*0.66+colors[i]*0.33;
		}
		
		// calculate error
		if (maxChan == 0) {
			err = abs(maxChanVal-(uint16)avg[1])+abs(maxChanVal-(uint16)avg[2]);
		} else if (maxChan == 1) {
			err = abs(maxChanVal-(uint16)avg[0])+abs(maxChanVal-(uint16)avg[2]);
		} else if (maxChan == 2) {
			err = abs(maxChanVal-(uint16)avg[0])+abs(maxChanVal-(uint16)avg[1]);
		}
		
		// bail out
		if (err < 4 || integr[0] >= 4095 || integr[1] >= 4095 || integr[2] >= 4095)
			break;
		
		for (i=0; i<3; i++) {
			if (i != maxChan) {
				if (abs(maxChanVal-(uint16)avg[i]) > 10) {
					step = 256;
				} else if (abs(maxChanVal-(uint16)avg[i]) > 5) {
					step = 32;
				} else {
					step = 16;
				}
				
				if (avg[i] < maxChanVal && integr[i]+step <= 4095) {
					integr[i] += step;
				} else if (avg[i] > maxChanVal && integr[i]-step >= 0) {
					integr[i] -= step;
				}
			}
		}
		
		adjdSetInt(integr[0], integr[1], integr[2], integr[3]);
	}
	
	return (uint16)((avg[0]+avg[1]+avg[2])/3.0);
}


uint16 adjdColorToRGB15(uint16 red, uint16 green, uint16 blue, uint16 level)
{
	uint8 temp[3];
	uint8 i;
	
	temp[0] = (uint8)((red*31.0)/level);
	temp[1] = (uint8)((green*31.0)/level);
	temp[2] = (uint8)((blue*31.0)/level);
	
	for (i=0; i<3; i++) {
		if (temp[i] > 31)
			temp[i] = 31;
	}
	
	return RGB15(temp[0], temp[1], temp[2]);
}
