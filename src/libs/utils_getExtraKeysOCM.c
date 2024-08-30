/*
-- Key matrix table
-- rows 0x0e(14) and 0x0f(15) from OCM Core

--  bit    7 F   6 E   5 D   4 C   3 B   2 A   1 9   0 8
--       +-----+-----+-----+-----+-----+-----+-----+-----+
--  ---- |ScrLk|     |     |     |     |     |     |     |  E
--       +-----+-----+-----+-----+-----+-----+-----+-----+
--  ---- | N/A |PrtSc|PgUp |PgDn | F9  | F10 | F11 | F12 |  F
--       +-----+-----+-----+-----+-----+-----+-----+-----+
-- bit     7 F   6 E   5 D   4 C   3 B   2 A   1 9   0 8
*/
#include <stdint.h>
#include "msx_const.h"


uint16_t getExtraKeysOCM() __naked __sdcccall(1)
{
	__asm
		di
		in   a, (0xaa)		; PPI-register A: Primary slot select register
		and  #0xf0

		or   #0x0e			; Row 'E' of Key matrix table
		out  (0xaa), a		; PPI-register C: Keyboard and cassette interface
		in   a, (0xa9)		; PPI-register B: Keyboard matrix row input register
		ld   e, a
							; Clear low nibble not needed, just to set bit #0
		or   #0x0f			; Row 'F' of Key matrix table
		out  (0xaa), a		; PPI-register C: Keyboard and cassette interface
		ei
		in   a, (0xa9)		; PPI-register B: Keyboard matrix row input register
		ld   d, a

		ret					; Returns DE: rows E (low nibble) & F (high nibble)
	__endasm;
}