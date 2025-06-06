/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "msx_const.h"
#include "conio.h"


#define DLG_MAX_TXT		24
#define DLG_MAX_BTN		4

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
	uint16_t *text;				// Dialog text lines (max:DLG_MAX_TXT) [NULL for last line]
	uint16_t *buttons;			// Button texts (max:DLG_MAX_BTN) [NULL for last button/no buttons]
	uint8_t defaultButton;		// Index for default button
	uint8_t cancelButton;		// Index for cancel button
	DlgAttrib_t attrib;			// Attributes bitmask
} Dialog_t;


// ========================================================
//  Functions

uint8_t showDialog(Dialog_t *dlg);
