#ifndef _REBOOT_H_
#define _REBOOT_H_


#ifdef ARM7

bool need_reboot();

#else

bool can_reboot();

#endif	// ARM7


void reboot();


#endif	// _REBOOT_H_
