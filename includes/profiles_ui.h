/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once
#include "types.h"


#define BTN_YES			0
#define BTN_NO			1
extern const uint16_t dlg_yesNoBtn[];

#define BTN_CONTINUE	0
extern const uint16_t dlg_continueBtn[];

#define BTN_CLOSE		0
extern const uint16_t dlg_closeBtn[];


void profiles_menu(Panel_t *panel);

