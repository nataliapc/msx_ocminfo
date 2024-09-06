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
#include "profiles_api.h"
#include "dialogs.h"


// ========================================================
// Variables

#define MAX_LINES	19

extern char *emptyArea;

static uint8_t *itemsCount;
static uint8_t topLine = 0, currentLine = 0;
static uint8_t newTopLine = 0, newCurrentLine = 0;
static bool end;


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
	{ "   Ok   ", NULL },
	0,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_saveChanges = {
	0,0,
	{ "Do you want to save changes?" },
	{ "  Yes  ", "  No   ", NULL },
	0,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_noProfiles = {
	0,0,
	{ "Profile list is empty!" },
	{ "   Ok   ", NULL },
	0,	//defaultButton
	0,	//cancelButton
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


// ========================================================
// Functions

void printHeader()
{
	// Clear panel
	textblink(1,3, 80, false);
	puttext(2,3, 79,23, emptyArea);

	// Function keys topbar
	putlinexy(4,3, 75, "F1:New   F2:Update                  F5:Delete  Ctrl+Up/Down:Order  BS:Back");

	// Elements panel
	chlinexy(2,4, 78);
}

void newProfile()
{


	//TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! editDialog to get description
	
	if (!*itemsCount) {
		currentLine--;
	}
	newCurrentLine = profile_newItem("Test profile name");
	if (newCurrentLine >= MAX_LINES) {
		newTopLine = newCurrentLine - MAX_LINES + 1;
		newCurrentLine -= newTopLine;
	}
}

updateProfile()
{
	profile_updateItem(topLine + currentLine);
}

deleteProfile()
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
}

void drawProfiles()
{
	ProfileItem_t *profile = profile_getItem(topLine);
	uint8_t count = profile_getHeaderData()->itemsCount;
	char lineStr[80];
	uint8_t i;

	if (count > MAX_LINES) count = MAX_LINES;
	for (i = 0; i < count; i++) {
		csprintf(lineStr, "%s", profile->description);
		putlinexy(3,5+i, strlen(lineStr), lineStr);
		profile++;
	}

	if (i <= MAX_LINES) {
		puttext(2,5+i, 79,23, emptyArea);
	}
}

void selectCurrentLine(bool enabled) {
	if (!*itemsCount && enabled) return;
	textblink(2,5+currentLine, 78, enabled);
}

void profiles_menu()
{
	itemsCount = &(profile_getHeaderData()->itemsCount);
	profile_init();

	// Checking for profile file & ask for creation if missing
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
	printHeader();
	drawProfiles();
	if (*itemsCount) {
		selectCurrentLine(true);
	}

	// Main loop profiles
	end = false;
	do {
		while (!kbhit()) {
			ASM_EI; ASM_HALT;
		}
		// Manage pressed key
		switch(getch()) {
			case KEY_UP:
				if (*itemsCount && topLine + currentLine > 0) {
					if (currentLine) {
						newCurrentLine--;
					} else {
						if (topLine) {
							newTopLine--;
						}
					}
				} else {
					putch('\x07');
				}
				break;
			case KEY_DOWN:
				if (*itemsCount && topLine + currentLine < *itemsCount - 1) {
					if (currentLine < MAX_LINES-1) {
						newCurrentLine++;
					} else {
						newTopLine++;
					}
				} else {
					putch('\x07');
				}
				break;
			case KEY_LEFT:
				if (topLine >= MAX_LINES) {
					newTopLine -= MAX_LINES;
				} else {
					newTopLine = 0;
					newCurrentLine = 0;
					putch('\x07');
				}
				break;
			case KEY_RIGHT:
				if (*itemsCount > MAX_LINES) {
					newTopLine += MAX_LINES;
					if (newTopLine + MAX_LINES > *itemsCount) {
						newTopLine = *itemsCount - MAX_LINES;
						newCurrentLine = MAX_LINES - 1;
						putch('\x07');
					}
				} else {
					newCurrentLine = *itemsCount - 1;
					putch('\x07');
				}
				break;
			case '1':
				newProfile();
				break;
			case '2':
				updateProfile();
				break;
			case '5':
				if (*itemsCount == 0) {
					showDialog(&dlg_noProfiles);
				} else {
					if (showDialog(&dlg_deleteProfile) == 0) {
						deleteProfile();
					}
				}
				break;
			case KEY_ESC:
			case KEY_BS:
				selectCurrentLine(false);
				end++;
				break;
		}
		if (topLine!=newTopLine || currentLine!=newCurrentLine) {
			selectCurrentLine(false);
			topLine = newTopLine;
			currentLine = newCurrentLine;
			drawProfiles();
			selectCurrentLine(true);
		}
		varPUTPNT = varGETPNT;
		varREPCNT = 0;
	} while (!end);

	if (showDialog(&dlg_saveChanges) == 0) {
		if (!profile_saveFile()) {
			showDialog(&dlg_errorSaving);
		}
	}

	profile_release();
}

