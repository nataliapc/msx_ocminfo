/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdio.h>
#include <string.h>
#include "msx_const.h"
#include "conio.h"
#include "heap.h"
#include "types.h"
#include "profiles_api.h"
#include "dialogs.h"


// ========================================================
// Defines

#define xstr(a) str(a)
#define str(a) #a

#define MAX_PROFILES	50
#define MAX_LINES		19

// ========================================================
// Variables

extern char *emptyArea;

static uint8_t *itemsCount;
static uint8_t editPanelIdx;
static uint8_t topLine = 0, currentLine = 0;
static uint8_t newTopLine = 0, newCurrentLine = 0;
static uint8_t key;
static bool redrawList, redrawSelection, doEditText;
static bool changedProfiles;
static bool end;

void drawProfiles();
void selectCurrentLine(bool enabled);

// ========================================================
// Constants

enum {
	PANEL_ADD,
	PANEL_UPDATE,
	PANEL_DELETE,
	PANEL_HELP,
	PANEL_PROFILES
};

static const Panel_t pPanels[] = {
	{ " [A]dd new ",	3,3, 	11 },
	{ " [U]pdate ",		14,3, 	10 },
	{ " [DEL]ete ",		24,3,	10 },
	{ " [H]elp ",		52,3,	8  },
	{ " [P]rofiles ",	60,3,	12 },
	{ " E[x]it ",		72,3,	8  },
	{ NULL }
};


// ========================================================
// Dialogs

Dialog_t dlg_fileNotFound = {
	0,0,
	{ "Profiles file not found", "or unreadable...", "", "Create a new file?" },
	{ "  Yes  ", "  No   ", NULL },
	0,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_errorSaving = {
	0,0,
	{ "Error saving profiles file!" },
	{ "  Close  ", NULL },
	0,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_noProfiles = {
	0,0,
	{ "Profile list is empty!" },
	{ "  Close  ", NULL },
	0,	//defaultButton
	0,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_saveChanges = {
	0,0,
	{ "Profiles modified.", "Do you want to save changes?" },
	{ "  Yes  ", "  No   ", NULL },
	0,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_deleteProfile = {
	0,0,
	{ "Remove selected profile?" },
	{ "  Yes  ", "  No   ", NULL },
	1,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_help = {
	0,0,
	{ "Up/Down . . . . Move selection       ",
	  "Right/Left. . . Next/Previous page   ",
	  "A . . . . . . . Add new profile      ",
	  "U . . . . . . . Update name & values ",
	  "DEL . . . . . . Delete selection     ",
	  "Ctrl+Up/Down. . Move selected item   ",
	  "H . . . . . . . Show this help       ",
	  "ESC/X . . . . . Exit profiles        "
	},
	{ "  Close  ", NULL },
	0,	//defaultButton
	0,	//cancelButton
	DLG_DEFAULT
};


// ========================================================
// Functions

void drawProfilesCounter()
{
	csprintf(heap_top, "\x13 %s%u/"xstr(MAX_PROFILES)" \x14",
		*itemsCount < 10 ? " ":"",
		*itemsCount);
	putlinexy(4,24, 9, heap_top);
}

void drawHeader()
{
	// Clear panel
	textblink(1,3, 80, false);
	puttext(2,3, 79,23, emptyArea);

	// Panel keys topbar
	Panel_t *panel = &pPanels[0];
	while (panel->title != NULL) {
		putlinexy(panel->titlex,panel->titley, panel->titlelen, panel->title);
		panel++;
	}

	// Elements panel
	chlinexy(2,4, 78);

	// Draw profiles counter
	drawProfilesCounter();
}

void beep()
{
	putch('\x07');
}

void editText(uint8_t itemNum)
{
	ProfileItem_t *item = profile_getItem(itemNum);
	bool end = false;
	uint8_t pos = strlen(item->description);

	putlinexy(5,newCurrentLine+5, 2, "<<");
	putlinexy(6+sizeof(item->description),newCurrentLine+5, 2, ">>");

	gotoxy(7+pos, newCurrentLine + 5);
	setcursortype(SOLIDCURSOR);

	do {
		key = getch();
		if (key == KEY_ENTER) {
			if (strlen(item->description) > 0) end++; else key = 0;
		} else
		if (key == KEY_DELETE || key == KEY_BS) {
			if (pos) {
				item->description[--pos] = '\0';
				putch(KEY_DELETE);
			} else key = 0;
		} else
		if (key >= 32 && key <= 254) {
			if (pos < sizeof(item->description)-1) {
				item->description[pos++] = key;
				putch(key);
			} else key = 0;
		} else {
			key = 0;
		}
		if (!key) beep();
	} while (!end);
	setcursortype(NOCURSOR);
}

void newProfile()
{
	if (!*itemsCount) {
		currentLine--;
	}
	newCurrentLine = profile_newItem();
	if (newCurrentLine >= MAX_LINES) {
		newTopLine = newCurrentLine - MAX_LINES + 1;
		newCurrentLine -= newTopLine;
	}
	doEditText++;
	redrawList++;
	drawProfilesCounter();
}

void updateProfile()
{
	profile_updateItem(topLine + currentLine);
	doEditText++;
	redrawList++;
}

void moveCurrentProfile(int8_t moveTo)
{
	ProfileItem_t *profile = profile_getItem(topLine+currentLine);
	memcpy(heap_top, profile+moveTo, sizeof(ProfileItem_t));
	memcpy(profile+moveTo, profile, sizeof(ProfileItem_t));
	memcpy(profile, heap_top, sizeof(ProfileItem_t));
	redrawList++;
	changedProfiles = true;
}

void deleteProfile()
{
	profile_deleteItem(topLine + currentLine);
	if (!currentLine) {
		if (topLine) {
			newTopLine--;
		}
	} else {
		if (topLine && topLine+(MAX_LINES-1) == *itemsCount) {
			newTopLine--;
		} else {
			newCurrentLine--;
		}
	}
	redrawList++;
	drawProfilesCounter();
}

void drawProfiles()
{
	ProfileItem_t *profile = profile_getItem(topLine);
	uint8_t count = *itemsCount;
	uint8_t i, num;

	if (count > MAX_LINES) count = MAX_LINES;
	num = topLine + 1;
	for (i = 0; i < count; i++, num++) {
		memset(heap_top, ' ', 78);					// Fill with spaces
		heap_top[0] = num < 10 ? ' ' : '0'+num/10;	// Order number
		heap_top[1] = '0'+num%10;
		memcpy(heap_top+4, 							// Profile description
			profile->description, 
			strlen(profile->description));
		csprintf(heap_top+66, "%u-%s%u-%s%u", 		// Date
			profile->modifYear,
			profile->modifMonth<10 ? "0":"", profile->modifMonth,
			profile->modifDay<10 ? "0":"", profile->modifDay);
		putlinexy(3,5+i, 76, heap_top);
		profile++;
	}

	if (i <= MAX_LINES) {
		puttext(2,5+i, 79,23, emptyArea);
	}
}

void selectCurrentLine(bool enabled)
{
	if (!*itemsCount && enabled) return;
	textblink(2,5+currentLine, 78, enabled);
}

void selectPanel(uint8_t idx, bool enabled)
{
	Panel_t *panel = &pPanels[idx];
	textblink(panel->titlex,panel->titley, panel->titlelen, enabled);
}

inline bool isCtrlKeyPressed()
{
	return varNEWKEY_row6.ctrl == 0;
}

void profiles_menu(Panel_t *panel)
{
	itemsCount = &(profile_getHeaderData()->itemsCount);
	profile_init();

	// Read profile file & ask for creation if missing or corrupted
	if (!profile_loadFile()) {
		if (showDialog(&dlg_fileNotFound) == 0) {
			profile_init();
			if (!profile_saveFile()) {
				showDialog(&dlg_errorSaving);
				return;
			}
		} else {
			return;
		}
	}

	// Initialize header & profiles
	newTopLine = newCurrentLine = topLine = currentLine = 0;
	changedProfiles = doEditText = false;
	drawHeader();
	textblink(panel->titlex, panel->titley, panel->titlelen, true);
	drawProfiles();
	if (*itemsCount) {
		selectCurrentLine(true);
	}

	// Main loop profiles
	end = false;
	do {
		while (!kbhit()) { ASM_EI; ASM_HALT; }
		// Manage pressed key
		key = getch();
		if (key == KEY_UP) {						// Move up selection
			if (*itemsCount && topLine + currentLine > 0) {
				if (currentLine) {
					newCurrentLine--;
					redrawSelection++;
				} else {
					if (topLine) {
						newTopLine--;
						redrawList++;
					}
				}
				if (isCtrlKeyPressed()) {
					moveCurrentProfile(-1);
				}
			} else {
				beep();
			}
		} else
		if (key == KEY_DOWN) {						// Move down selection
			if (*itemsCount && topLine + currentLine < *itemsCount - 1) {
				if (currentLine < MAX_LINES-1) {
					newCurrentLine++;
					redrawSelection++;
				} else {
					newTopLine++;
					redrawList++;
				}
				if (isCtrlKeyPressed()) {
					moveCurrentProfile(1);
				}
			} else {
				beep();
			}
		} else
		if (key == KEY_LEFT) { 						// Previous page
			if (topLine >= MAX_LINES) {
				newTopLine -= MAX_LINES;
			} else {
				newTopLine = 0;
				newCurrentLine = 0;
				beep();
			}
			redrawList++;
		} else
		if (key == KEY_RIGHT) {						// Next page
			if (*itemsCount > MAX_LINES) {
				newTopLine += MAX_LINES;
				if (newTopLine + MAX_LINES > *itemsCount) {
					newTopLine = *itemsCount - MAX_LINES;
					newCurrentLine = MAX_LINES - 1;
					beep();
				}
			} else {
				newCurrentLine = *itemsCount - 1;
				beep();
			}
			redrawList++;
		} else
		if (key == 'a' || key == 'A') {				// Add new profile
			if (*itemsCount < MAX_PROFILES) {
				editPanelIdx = PANEL_ADD;
				newProfile();
			} else {
				beep();
			}
		} else
		if (key =='u' || key == 'U') {				// Update selection
			if (*itemsCount == 0) {
				showDialog(&dlg_noProfiles);
			} else {
				editPanelIdx = PANEL_UPDATE;
				updateProfile();
			}
		} else 
		if (key == KEY_DELETE) {					// Delete selection
			if (*itemsCount == 0) {
				showDialog(&dlg_noProfiles);
			} else {
				selectPanel(PANEL_DELETE, true);
				if (showDialog(&dlg_deleteProfile) == 0) {
					deleteProfile();
					changedProfiles = true;
				}
				selectPanel(PANEL_DELETE, false);
			}
			redrawList++;
		} else
		if (key == KEY_ENTER) {						// Apply current profile
			// TODO !!!!!!!!!!!!!!!!!!! apply current profile
			beep();
		} else
		if (key == 'h' || key == 'H') {				// Show help dialog
			showDialog(&dlg_help);
			redrawSelection++;
		} else
		if (key == KEY_ESC || key == KEY_BS ||		// Exit profiles
			key == 'x' || key == 'X') {
			end++;
		}
		if (redrawList || redrawSelection) {
			selectCurrentLine(false);
			topLine = newTopLine;
			currentLine = newCurrentLine;
			if (redrawList) drawProfiles();
			if (*itemsCount) selectCurrentLine(true);
			redrawList = redrawSelection = false;
		}
		if (doEditText) {
			selectPanel(editPanelIdx, true);
			editText(topLine + currentLine);
			selectPanel(editPanelIdx, false);
			drawProfiles();
			doEditText = false;
			changedProfiles = true;
		}
		varPUTPNT = varGETPNT;
		varREPCNT = 0;
	} while (!end);

	selectCurrentLine(false);

	if (changedProfiles && showDialog(&dlg_saveChanges) == 0) {
		if (!profile_saveFile()) {
			showDialog(&dlg_errorSaving);
		}
	}

	profile_release();
	textblink(panel->titlex, panel->titley, panel->titlelen, false);
}

