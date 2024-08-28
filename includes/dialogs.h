#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "msx_const.h"
#include "conio.h"


// ========================================================
//  Structs & Enums

typedef enum {
	DLG_DEFAULT = 0,
//	DLG_TXT_ALIGN_MASK   = 0x03,	// Texts alignment:   0b00000011
//	DLG_TXT_ALIGN_CENTER = 0x00,
//	DLG_TXT_ALIGN_LEFT   = 0x01,
//	DLG_TXT_ALIGN_RIGHT  = 0x02,
//	DLG_BTN_ALIGN_MASK   = 0x0c,	// Buttons alignment:    0b00001100
//	DLG_BTN_ALIGN_CENTER = 0x00,
//	DLG_BTN_ALIGN_LEFT   = 0x04,
//	DLG_BTN_ALIGN_RIGHT  = 0x08,
} DlgAttrib_t;

typedef struct {
	uint8_t posX, posY;			// 0,0 -> dialog centered
	char *text[6];				// Dialog text lines (max: 5) [NULL for last line]
	char *buttons[4];			// Button texts (max:3) [NULL for last button]
	uint8_t defaultButton;		// Index for default button
	uint8_t cancelButton;		// Index for cancel button
	DlgAttrib_t attrib;			// Attributes bitmask
} Dialog_t;


// ========================================================
//  Functions

uint8_t showDialog(Dialog_t *dlg);
