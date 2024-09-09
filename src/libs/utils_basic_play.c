#include "msx_const.h"
#include "utils.h"


void basic_play(void *params) __naked __sdcccall(1)
{
	params;
	__asm
		push ix
		push hl										; Store the Param 'params'

		ld   hl, #.play_call						; Copy routine to _heap_top
		ld   de, (_heap_top)						; _heap_top must be >= 0x8000
		push de
		pop  ix
		ld   bc, #.play_call_end - .play_call
		ldir

		pop  hl										; Copy the PLAY sentence from 'params'
		push de										; Store the start of PLAY sentence
		ld   bc, #80								; Max length of PLAY sentence
		ldir

		jp   (ix)

	.play_call:
		pop  hl										; Recover the start of PLAY sentence

		di											; Store current slot configuration
		in   a,(0xa8)
		push af
		and  #0b11110000							; Set ROMs pagination
		ei
		out  (0xa8),a		// TODO: take care of subslots

							// TODO: hardcoded. Read (0x39AE) to get the PLAY address
		call 0x73e5									; HL = parameters for PLAY as Asciiz

	.wait_loop:
		ld   hl, #VCBA
		ld   a, (hl)
		inc  hl
		or   (hl)
		ld   hl, #VCBB
		or   (hl)
		inc  hl
		or   (hl)
		ld   hl, #VCBC
		or   (hl)
		inc  hl
		or   (hl)
		jr   nz, .wait_loop

		pop  af
		di
		ei
		out  (0xa8),a		// TODO: take care of subslots

		pop  ix
		ret

	.play_call_end:
	.play_sentence:
	__endasm;
}
