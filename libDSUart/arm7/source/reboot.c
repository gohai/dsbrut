#include <nds.h>
#include "reboot.h"

// code taken from rebootlib 1.1 by Rick Wong (Lick)
// works for SuperKey only


bool need_reboot()
{
	return (*(vu32*)0x27FFE24 == 0x27FFE04);
}


void reboot()
{
    irqDisable(IRQ_ALL);
    *(vu32*)0x27FFE34 = *(vu32*)0x27FFFF8;
    swiSoftReset();
}
