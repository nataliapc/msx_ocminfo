/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdio.h>
#include <string.h>
#include "msx_const.h"
#include "conio.h"
#include "dos.h"
#include "heap.h"
#include "utils.h"
#include "globals.h"
#include "types.h"
#include "ocm_ioports.h"
#include "profiles_ui.h"
#include "profiles_api.h"
#include "dialogs.h"
#include "strings_index.h"


// ========================================================
// Defines

#define xstr(a) str(a)
#define str(a) #a

#define MAX_LINES		15

#define NO_LOG			-1

// ========================================================
// Variables

extern char *emptyArea;


static uint8_t *itemsCount;
static uint8_t editPanelIdx;
static uint8_t topLine = 0, currentLine = 0;
static uint8_t newTopLine = 0, newCurrentLine = 0;
static uint16_t logIdx = NO_LOG;
static uint8_t key;
static bool redrawList, redrawSelection, doEditText;
static bool changedProfiles;
static bool end;

void beep_ok();
void beep_advice();
void beep_fail();
void beep_error();
void putstrxy(uint8_t x, uint8_t y, char *str);

void setByteVRAM(uint16_t vram, uint8_t value) __sdcccall(0);
void _fillVRAM(uint16_t vram, uint16_t len, uint8_t value) __sdcccall(0);

void drawProfiles();
void selectCurrentLine(bool enabled);

// ========================================================
// Constants

enum {
	PANEL_ADD,
	PANEL_UPDATE,
	PANEL_DELETE,
	PANEL_HELP,
	PANEL_PROFILES,
	PANEL_BACK
};

static const Panel_t pPanels[] = {
	{ MENUPROF_ADDNEW,		2,3, 	11 },
	{ MENUPROF_UPDATE,		13,3, 	10 },
	{ MENUPROF_DELETE,		23,3,	10 },
	{ MENUPROF_HELP,		33,3,	8  },
	{ MENUPROF_PROFILES,	61,3,	12 },
	{ MENUPROF_BACK,		72,3,	8  },
	{ ARRAYEND }
};


// ========================================================
// Dialogs

const uint16_t dlg_yesNoBtn[] = { DLG_BTN_YES, DLG_BTN_NO, ARRAYEND };
const uint16_t dlg_continueBtn[] = { DLG_BTN_CONTINUE, ARRAYEND };
const uint16_t dlg_closeBtn[] = { DLG_BTN_CLOSE, ARRAYEND };

const uint16_t dlg_fileNotFoundStr[] = {
	DLG_FILENOTFOUND_TITLE, DLG_FILENOTFOUND_TEXT1, DLG_FILENOTFOUND_TEXT2, DLG_FILENOTFOUND_TEXT3, ARRAYEND
};
const Dialog_t dlg_fileNotFound = {
	0,0,
	dlg_fileNotFoundStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_NO,		//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_errorSavingStr[] = { DLG_ERRORSAVINGPROFILE_TITLE, ARRAYEND };
const Dialog_t dlg_errorSaving = {
	0,0,
	dlg_errorSavingStr,
	dlg_continueBtn,
	BTN_CONTINUE,	//defaultButton
	BTN_CONTINUE,	//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_noProfilesStr[] = { DLG_NOPROFILES_TITLE, ARRAYEND };
const Dialog_t dlg_noProfiles = {
	0,0,
	dlg_noProfilesStr,
	dlg_continueBtn,
	BTN_CONTINUE,	//defaultButton
	BTN_CONTINUE,	//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_saveChangesStr[] = { DLG_SAVECHANGES_TITLE, DLG_SAVECHANGES_TEXT1, ARRAYEND };
const Dialog_t dlg_saveChanges = {
	0,0,
	dlg_saveChangesStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_NO,		//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_deleteProfileStr[] = { DLG_DELETEPROFILE_TITLE, ARRAYEND };
const Dialog_t dlg_deleteProfile = {
	0,0,
	dlg_deleteProfileStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_NO,		//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_profileAppliedStr[] = { DLG_PROFILEAPPLIED_TITLE, ARRAYEND };
const Dialog_t dlg_profileApplied = {
	0,0,
	dlg_profileAppliedStr,
	dlg_continueBtn,
	BTN_CONTINUE,	//defaultButton
	BTN_CONTINUE,	//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_mutedSoundStr[] = { DLG_MUTESOUND_TITLE, ARRAYEND };
const Dialog_t dlg_mutedSound = {
	0,0,
	dlg_mutedSoundStr,
	dlg_continueBtn,
	BTN_CONTINUE,	//defaultButton
	BTN_CONTINUE,	//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_unmutedSoundStr[] = { DLG_UNMUTESOUND_TITLE, ARRAYEND };
const Dialog_t dlg_unmutedSound = {
	0,0,
	dlg_unmutedSoundStr,
	dlg_continueBtn,
	BTN_CONTINUE,	//defaultButton
	BTN_CONTINUE,	//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_helpStr[] = {
	DLG_PROFILESHELP_TITLE, DLG_PROFILESHELP_TEXT1, DLG_PROFILESHELP_TEXT2, DLG_PROFILESHELP_TEXT3,
	DLG_PROFILESHELP_TEXT4, DLG_PROFILESHELP_TEXT5, DLG_PROFILESHELP_TEXT6, DLG_PROFILESHELP_TEXT7,
	DLG_PROFILESHELP_TEXT8, DLG_PROFILESHELP_TEXT9, ARRAYEND
};
const Dialog_t dlg_help = {
	0,0,
	dlg_helpStr,
	dlg_closeBtn,
	BTN_CLOSE,		//defaultButton
	BTN_CLOSE,		//cancelButton
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
	waitVBLANK();

	// Clear panel
	textblink(1,3, 80, false);
	puttext(2,3, 79,23, emptyArea);

	// Panel keys topbar
	Panel_t *panel = &pPanels[0];
	while (panel->title != ARRAYEND) {
		putlinexy(panel->titlex,panel->titley, panel->titlelen, getString(panel->title));
		panel++;
	}

	// Elements panel
	chlinexy(2,4, 78);

	// Log panel
	chlinexy(2,5+MAX_LINES, 78);
	setByteVRAM(2+22*80, '>');

	// Draw profiles counter
	drawProfilesCounter();
}

void scrollupLog()
{
	gettext(5,7+MAX_LINES, 79,23, heap_top);
	puttext(5,6+MAX_LINES, 79,22, heap_top);
	_fillVRAM(4+22*80, 75, ' ');
}

void printLog(uint16_t log)
{
	scrollupLog();
	putstrxy(5,23, getString(log));
}

void printLogIdx(uint16_t logPattern)
{
	char *ptr = malloc(80);
	csprintf(ptr, getString(logPattern), topLine + currentLine + 1);
	scrollupLog();
	putstrxy(5,23, ptr);
	free(80);
}

// ========================================================
inline bool isCtrlKeyPressed()
{
	return varNEWKEY_row6.ctrl == 0;
}

// ========================================================
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
		if (!key) beep_fail();
	} while (!end);
	setcursortype(NOCURSOR);
}

// ========================================================
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

void applyProfileCmds()
{
	uint8_t *cmd = profile_getItem(topLine+currentLine)->cmd;
	while (*cmd) {
		ocm_sendSmartCmd(*cmd++);
	}
}

// ========================================================
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
		puttext(2,5+i, 79,4+MAX_LINES, emptyArea);
	}
}

void selectCurrentLine(bool enabled)
{
	if (!*itemsCount && enabled) return;
	textblink(2,5+currentLine, 78, enabled);
}

void selectPanel(uint8_t idx, bool enabled)
{
	Panel_t *panel = &pPanels[enabled ? PANEL_PROFILES : idx];
	textblink(panel->titlex,panel->titley, panel->titlelen, false);
	panel = &pPanels[enabled ? idx : PANEL_PROFILES];
	textblink(panel->titlex,panel->titley, panel->titlelen, true);
}

void showDialogNoProfiles()
{
	beep_error();
	showDialog(&dlg_noProfiles);
}

// ========================================================
void profiles_menu(Panel_t *panel)
{
	itemsCount = &(profile_getHeaderData()->itemsCount);
	profile_init();

	// Initialize header & profiles
	newTopLine = newCurrentLine = topLine = currentLine = 0;
	changedProfiles = doEditText = false;
	drawHeader();
	textblink(panel->titlex, panel->titley, panel->titlelen, true);

	// Read profile file & ask for creation if missing or corrupted
	printLog(LOG_PROF_READINGFILE);
	if (!profile_loadFile()) {
		printLog(LOG_PROF_NOTFOUND);
		beep_fail();
		if (showDialog(&dlg_fileNotFound) == BTN_YES) {
			profile_init();
			if (!profile_saveFile()) {
				printLog(LOG_PROF_CANTCREATEFILE);
				beep_error();
				showDialog(&dlg_errorSaving);
				goto end_profile_menu;
			}
		} else {
			goto end_profile_menu;
		}
		printLog(LOG_PROF_FILECREATED);
	} else {
		printLog(LOG_PROF_FILEREADED);
	}

	// Draw profiles & select first line if any
	drawProfiles();
	drawProfilesCounter();
	if (*itemsCount) {
		selectCurrentLine(true);
	}

	// Main loop profiles list
	end = false;
	do {
		while (!kbhit()) { waitVBLANK(); }
		// Manage pressed key
		key = dos2_toupper(getch());
		if (key == KEY_UP) {						// Move up selection
			if (*itemsCount) {
				if (topLine + currentLine > 0) {
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
						logIdx = LOG_PROF_MOVEDUP;
					}
				} else beep_fail();
			} else {
				if (isCtrlKeyPressed())
					showDialogNoProfiles();
				else
					beep_fail();
			}
		} else
		if (key == KEY_DOWN) {						// Move down selection
			if (*itemsCount) {
				if (topLine + currentLine < *itemsCount - 1) {
					if (currentLine < MAX_LINES-1) {
						newCurrentLine++;
						redrawSelection++;
					} else {
						newTopLine++;
						redrawList++;
					}
					if (isCtrlKeyPressed()) {
						moveCurrentProfile(1);
						logIdx = LOG_PROF_MOVEDOWN;
					}
				} else beep_fail();
			} else {
				if (isCtrlKeyPressed())
					showDialogNoProfiles();
				else
					beep_fail();
			}
		} else
		if (key == KEY_LEFT) { 						// Previous page
			if (topLine >= MAX_LINES) {
				newTopLine -= MAX_LINES;
			} else {
				newTopLine = 0;
				newCurrentLine = 0;
				beep_fail();
			}
			redrawList++;
		} else
		if (key == KEY_RIGHT) {						// Next page
			if (*itemsCount > MAX_LINES) {
				newTopLine += MAX_LINES;
				if (newTopLine + MAX_LINES > *itemsCount) {
					newTopLine = *itemsCount - MAX_LINES;
					newCurrentLine = MAX_LINES - 1;
					beep_fail();
				}
			} else {
				newCurrentLine = *itemsCount - 1;
				beep_fail();
			}
			redrawList++;
		} else
		if (key == 'A') {							// Add new profile
			if (*itemsCount < MAX_PROFILES) {
				editPanelIdx = PANEL_ADD;
				newProfile();
				logIdx = LOG_PROF_ADDEDNEW;
			} else {
				printLog(LOG_PROF_LIMITERROR);
				beep_error();
			}
		} else
		if (key == 'U') {							// Update selection
			if (*itemsCount == 0) {
				showDialogNoProfiles();
			} else {
				editPanelIdx = PANEL_UPDATE;
				updateProfile();
				logIdx = LOG_PROF_UPDATED;
			}
		} else 
		if (key == KEY_DELETE) {					// Delete selection
			if (*itemsCount == 0) {
				showDialogNoProfiles();
			} else {
				selectPanel(PANEL_DELETE, true);
				if (showDialog(&dlg_deleteProfile) == BTN_YES) {
					deleteProfile();
					printLogIdx(LOG_PROF_DELETED);
					changedProfiles = true;
				}
				selectPanel(PANEL_DELETE, false);
			}
			redrawList++;
		} else
		if (key == KEY_ENTER) {						// Apply current profile
			if (*itemsCount == 0) {
				showDialogNoProfiles();
			} else {
				applyProfileCmds();
				beep_ok();
				printLogIdx(LOG_PROF_APPLIED);
				showDialog(&dlg_profileApplied);
				redrawSelection++;
			}
		} else
		if (key == 'M') {							// Mute/unmute menu sounds
			ProfileHeaderData_t *headerData = profile_getHeaderData();
			headerData->muteSound = !(headerData->muteSound);
			changedProfiles = true;
			beep_ok();
			showDialog(headerData->muteSound ? &dlg_mutedSound : &dlg_unmutedSound);
		} else
		if (key == 'H') {							// Show help dialog
			selectPanel(PANEL_HELP, true);
			showDialog(&dlg_help);
			selectPanel(PANEL_HELP, false);
			redrawSelection++;
		} else
		if (key == KEY_ESC || key == 'B') {			// Go back to panels
			end++;
		}
		// Update selection or full list if necessary
		if (redrawList || redrawSelection) {
			waitVBLANK();
			selectCurrentLine(false);
			topLine = newTopLine;
			currentLine = newCurrentLine;
			if (redrawList) drawProfiles();
			if (*itemsCount) selectCurrentLine(true);
			redrawList = redrawSelection = false;
			if (logIdx != NO_LOG) {
				printLogIdx(logIdx);
				logIdx = NO_LOG;
			}
		}
		// Handle description editing if necessary
		if (doEditText) {
			printLogIdx(LOG_PROF_EDITING);
			selectPanel(editPanelIdx, true);
			beep_advice();
			editText(topLine + currentLine);
			selectPanel(editPanelIdx, false);
			drawProfiles();
			printLog(LOG_PROF_MODIFIED);
			beep_advice();
			doEditText = false;
			changedProfiles = true;
		}
		varPUTPNT = varGETPNT;
		varREPCNT = 0;
	} while (!end);

	// Unselect current selection
	selectCurrentLine(false);

	// Check if profiles have changed and prompt for saving
	if (changedProfiles) {
		selectPanel(PANEL_BACK, true);
		if (showDialog(&dlg_saveChanges) == BTN_YES) {
			printLog(LOG_PROF_SAVINGCFG);
			if (!profile_saveFile()) {
				showDialog(&dlg_errorSaving);
			}
		}
		selectPanel(PANEL_BACK, false);
	}

	// Release memory & clear panel selection
end_profile_menu:
	profile_release();
	textblink(panel->titlex, panel->titley, panel->titlelen, false);
}

