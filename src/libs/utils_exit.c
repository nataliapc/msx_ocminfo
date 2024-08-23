#include "utils.h"
#include "msx_const.h"


/*
    PROGRAM TERMINATE (00H)
    Parameters:    C = 00H (_TERM0)
    Results:       Does not return

This function terminates program with a zero return code. It is provided for
compatibility with MSX-DOS 1 and CP/M, the preferred method of exiting a
program is to use the "terminate with error code" function call (function
62h), passing a zero error code if that is what is desired. See the description
of that function call, and also the "Program Interface Specification", for
details of what happens when a program terminates. This function call never
returns to the caller.
*/
void exit(void) __naked
{
	__asm
		ld c,#TERM0
		DOSJP
	__endasm;
}
