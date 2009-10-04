#ifndef _TIME7_H_
#define _TIME7_H_


/**
 *	this file is only needed when you want to use libdsuart on the 
 *	arm7 cpu.
 */


#include <time.h>
#include "nds/system.h"


#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

static const short ydays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };


/**
 *	return the current system time on the arm7 cpu.
 */
static inline time_t time_arm7()
{
	RTCtime dstime;
	int years, months, days, hours, minutes, seconds;
	
	rtcGetTimeAndDate((uint8*)&dstime);
	
	// taken from __mktime()
	years = dstime.year + 2000;
	months = dstime.month -1;
	days = dstime.day - 1;
	hours = dstime.hours;
	minutes = dstime.minutes;
	seconds = dstime.seconds;
	days += ydays[months] + (months > 1 && leap (years));
	days = (unsigned)days + 365 * (unsigned)(years - 1970) + (unsigned)(nleap (years));

	return (time_t)(86400L * (unsigned long)days + 3600L * (unsigned long)hours + (unsigned long)(60 * minutes + seconds));
}


#endif	// _TIME7_H__