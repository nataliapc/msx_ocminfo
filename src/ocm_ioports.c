/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include "ocm_ioports.h"


bool ocm_detectDevice(uint8_t devId) __naked __z88dk_fastcall
{
	devId;								// L = Param devId
	__asm
		in   a, (0x40)					; backup current manufacturer/device
		cpl
		push af

		call .detectExtIODevice
		ld   l, a						; Returns L = 0:false 1:true

	.odv_end:
		pop  af							; restore original manufacturer/device
		out  (0x40), a
		ret

	.detectExtIODevice:					; IN: L=DeviceID | OUT: A=0:false 1:true
		ld   a, l
		out  (0x40), a					; out the manufacturer/device code to I/O port 0x40
		in   a, (0x40)					; read the value you have just written
		cpl								; complement all bits of the value
		cp   l							; if it does not match the value you originally wrote,

		ld   a, #0
		ret  nz							; it does not exists on expanded I/O ports
		inc  a
		ret								; does exists
	__endasm;
}

uint8_t ocm_getPortValue(uint8_t port) __naked __z88dk_fastcall
{
	port;								// L = Param port
	__asm
		in   a, (0x40)					; backup current manufacturer/device
		cpl
		push af

		ld   c, l
		ld   l, #0xd4					; DEVID_OCMPLD
		call .detectExtIODevice
		ld   l, a
		jr   z, .odv_end				; Return if not detected

		in a, (c)						; Read the OCM device port
		ld l, a							; Returns L = ext I/O port value

		jr .odv_end
	__endasm;
}

uint16_t ocm_getDynamicPortValue(uint8_t index) __naked __z88dk_fastcall
{
	index;
	__asm
		in   a, (0x40)					; backup current manufacturer/device
		cpl
		push af
		push hl							; backup 'index' param (L)

		ld   c, l
		ld   hl, #0xffd4				; H=Error flag | L=DEVID_OCMPLD
		call .detectExtIODevice
		ld   l, a
		pop  de							; Recover 'index' param (E)
		jr   z, .odv_end				; Return if not detected

		ld   a, e						; Write_neg the 'index' to 0x44
		cpl
		out  (0x44), a
		in   a, (0x44)
		cp   e							; Return error if 'index' not supported
		jr   nz, .odv_end

		inc  h							; set Error flag to false(0)
		in   a, (0x4b)
		ld   l, a

		jr .odv_end						; Returns HL = ext I/O dinamic port value
	__endasm;
}

bool ocm_sendSmartCmd(uint8_t cmd) __naked __z88dk_fastcall
{
	cmd;								// L = Param cmd
	__asm
		in a, (0x40)					; backup current manufacturer/device
		cpl
		push af

		ld b, l							; Store Param cmd
		ld l, #0xd4						; DEVID_OCMPLD
		call .detectExtIODevice

		ld l, a
		jr z, .odv_end

		ld l, a							; Returns L = 0:fail 1:success
		ld a, b							; Restore Param cmd
		out (0x41), a					; send smart command to OCM_SMARTCMD_PORT

		jr .odv_end
	__endasm;
}