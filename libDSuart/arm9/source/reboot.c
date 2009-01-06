#include <nds.h>
#include <sys/stat.h>
#include "reboot.h"

// code taken from rebootlib 1.1 by Rick Wong (Lick)
// works for SuperKey only

#define DEVICE_SCCF (0x46434353)
#define DEVICE_SCSD (0x44534353)
#define DEVICE_SCLT (0x544C4353)


bool can_reboot()
{
	struct stat st;
	stat("/.", &st);
	unsigned int device = st.st_dev;
	
	if (device == DEVICE_SCCF || device == DEVICE_SCSD || DEVICE_SCLT)
		return true;
	else
		return false;
}


void reboot()
{
    irqDisable(IRQ_ALL);
    sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);
	
	// code for SuperKey
	*(vu16*)0x09FFFFFE = 0xA55A;
    *(vu16*)0x09FFFFFE = 0xA55A;
    *(vu16*)0x09FFFFFE = 0;
    *(vu16*)0x09FFFFFE = 0;
	*(vu32*)0x27FFFF8 = 0x8000000;
	
    *(vu32*)0x27FFFFC = 0;
    *(vu32*)0x27FFE04 = 0xE59FF018;
    *(vu32*)0x27FFE24 = 0x27FFE04;
    sysSetBusOwners(BUS_OWNER_ARM7, BUS_OWNER_ARM7);
    swiSoftReset();
}
