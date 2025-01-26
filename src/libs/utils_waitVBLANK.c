#include "msx_const.h"


void waitVBLANK()
{
	uint16_t lastJIFFY = varJIFFY;
	do {
		ASM_EI; ASM_HALT;
	} while (lastJIFFY == varJIFFY);
}