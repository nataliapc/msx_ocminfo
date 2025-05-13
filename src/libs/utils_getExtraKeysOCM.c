/*
-- Key matrix table
-- rows 0x0e(14) and 0x0f(15) from OCM Core

--  bit    7 F   6 E   5 D   4 C   3 B   2 A   1 9   0 8
--       +-----+-----+-----+-----+-----+-----+-----+-----+
--  ---- |ScrLk|     |     |     |     |     |     |     |  E
--       +-----+-----+-----+-----+-----+-----+-----+-----+
--  ---- | N/A |PrtSc|PgUp |PgDn | F9  | F10 | F11 | F12 |  F
--       +-----+-----+-----+-----+-----+-----+-----+-----+
--  bit    7 F   6 E   5 D   4 C   3 B   2 A   1 9   0 8
*/
#include <stdint.h>
#include "msx_const.h"


uint16_t getExtraKeysOCM() __naked __sdcccall(1)
{
	__asm
		di
		push bc
		// Get row E
		ld   b, #0x0e		; Row 'E' of Key matrix table
		call get_row
		ld   e, a
		// Get row F
		inc  b				; Row 'F' of Key matrix table
		call get_row
		ld   d, a
		pop  bc
		ei
		ret					; Returns DE: E=row E, D=row F

	get_row:	; IN: b = row number | OUT: a = row value
		in   a, (0xaa)		; in  PPI-register C: Keyboard and cassette interface
		and  #0xf0
		add  a, b
		out  (0xaa), a		; out PPI-register C: Keyboard and cassette interface
		in   a, (0xa9)		; in  PPI-register B: Keyboard matrix row input register
		ret

	__endasm;
}