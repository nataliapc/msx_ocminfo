#include <stdint.h>
#include "utils.h"
#include "msx_const.h"


uint8_t getRomByte(uint16_t address) __naked __sdcccall(1)
{
	address;
	__asm
		push ix			; HL = param address
		xor a			; Not expanded, Primary Slot0, Secondary Slot 0
		call  RDSLT
		pop ix
		reti			; Returns A = byte content
	__endasm;
}