/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "msx_const.h"
#include "dos.h"
#include "conio.h"
#include "utils.h"
#include "globals.h"
#include "ocm_ioports.h"
#include "profiles_api.h"
#include "strings_index.h"


// ========================================================
static ProfileItem_t *item;
static bool verbose = true;

static const char btmFile1[] =
	"set SMARTDIR=A:\\UTILS\\SETSMART.COM\n"
	"if not exist %SMARTDIR% echo *** %SMARTDIR% not found, edit this file and correct the path\n"
	"if not exist %SMARTDIR% goto ~exit\n"
	"set SMARTCMD=";
static const char btmFile2[] =
	"\n"
	"echo setsmart #%SMARTCMD%\n"
	"%SMARTDIR% #%SMARTCMD%\n"
	"~exit\n"
	"set SMARTDIR=\n"
	"set SMARTCMD=\n";


// ========================================================
void printHelp()
{
	cputs(
		"https://github.com/nataliapc/msx_ocminfo\n"
		"\n"
		"Usage: OCMINFO [/n|/L] [/B] [/R] [/Q] [/?]\n"
		"\n"
		"Use without parameters to open the interactive panels mode.\n"
		"\n"
		"Options:\n"
		"  /n    Apply the user profile 'n'.\n"
		"  /L    List the user profiles.\n"
		"  /B    Print a .BTM file for the selected profile.\n"
		"  /R    Reset OCM to default values.\n"
		"  /Q    Quiet mode (no verbose).\n"
		"  /?    Show this help.\n"
		"\n"
		"Examples:\n"
		"  OCMINFO         Opens the interactive panels mode.\n"
		"  OCMINFO /L      List the available profiles.\n"
		"  OCMINFO /1      Apply the user profile #1\n"
		"  OCMINFO /1 /B > PROFILE1.BTM\n"
		"                  Creates a .BTM file to flash profile #1 into EPCS.\n"
	);
}

bool readProfilesFile()
{
	if (profile_loadFile()) {
		return true;
	}
	cputs(getString(CMD_ERROR_NOTFOUND));
	return false;
}

void doResetToDefaults()
{
	if (verbose) cputs(getString(CMD_RESET));
	ocm_sendSmartCmd(OCM_SMART_ResetDefaults);
}

void doListProfiles()
{
	uint8_t idx = 0;

	if (profile_loadFile()) {
		if (profile_getHeaderData()->itemsCount) {
			while (item = profile_getItem(idx++)) {
				cprintf("%u: %s\n", idx, item->description);
				if (idx % (varCRTCNT-4) == 0) {
					cputs(getString(CMD_PRESS_A_KEY));
					getch();
					cputs("\r"VT_CLREOL);
				}
			}
		} else {
			cputs(getString(CMD_ERROR_NOITEMS));
		}
	}
}

bool getProfile(uint8_t idx)
{
	if (profile_loadFile()) {
		if (item = profile_getItem(idx-1)) {
			return true;
		}
	}
	cprintf(getString(CMD_ERROR_NOPROFILE), idx);
	return false;
}

void doApplyProfile(uint8_t idx)
{
	if (getProfile(idx)) {
		uint8_t *cmd = item->cmd;
		if (verbose) {
			cprintf(getString(CMD_APPLYING), idx, item->description);
		}
		while (*cmd) {
			ocm_sendSmartCmd(*cmd);
			if (verbose) {
				if (*cmd < 16) putch('0');
				cprintf("%x", *cmd);
			}
			cmd++;
		}
	}
}

void doPrintBTMFile(uint8_t idx)
{
	if (getProfile(idx)) {
		uint8_t *cmd = item->cmd;

		cputs(btmFile1);
		while (*cmd) {
			if (*cmd < 16) putch('0');
			cprintf("%x", *cmd);
			cmd++;
		}
		cputs(btmFile2);
	}
}

uint16_t commandLine(char **argv, int argc)
{
	bool showHelp = true;
	uint16_t profileToApply = 0;
	bool listProfiles = false;
	bool paramDetected = false;
	bool resetDetected = false;
	bool btmDetected = false;
	uint8_t i = 0;
	char *arg;

	while(i < argc) {
		arg = argv[i++];
		arg[1] = dos2_toupper(arg[1]);
		if (*arg++ != '/') {
			showHelp = true;
			break;
		} else
		if (*arg == 'B') {				// '/B'
			btmDetected++;
			verbose = false;
			showHelp = false;
		} else
		if (*arg == 'R') {				// '/R'
			resetDetected++;
			showHelp = false;
		} else
		if (*arg =='Q') {				// '/Q'
			if (!listProfiles) {
				verbose = false;
			}
		} else
		if (!paramDetected) {
			if (*arg =='L') {			// '/L'
				listProfiles++;
				paramDetected++;
				showHelp = false;
			} else
			if (isdigit(*arg)) {		// '/n'
				do {
					profileToApply *= 10;
					profileToApply += *arg - '0';
					if (!isdigit(*arg) || profileToApply>MAX_PROFILES) {
						profileToApply = 0;
						break;
					}
				} while(*(++arg));
				if (!profileToApply) {
					cputs(getString(CMD_ERROR_INVALID));
					break;
				}
				paramDetected++;
				showHelp = false;
			}
		} else {
			showHelp = true;
			break;
		}
	}

	if (btmDetected && (!profileToApply || resetDetected || listProfiles)) {
		showHelp = true;
	}

	if (showHelp || verbose) {
		cprintf(getString(CMD_HEADER), getString(HEADER_VERSION), getString(HEADER_AUTHOR));
	}

	if (showHelp) {
		printHelp();
	} else {
		if (verbose) cputs("\n");
		if (resetDetected) {
			doResetToDefaults();
		}
		if (listProfiles) {
			doListProfiles();
		} else
		if (profileToApply) {
			if (!btmDetected) {
				doApplyProfile((uint8_t)profileToApply);
			} else {
				doPrintBTMFile((uint8_t)profileToApply);
			}
		}
	}

	return 1;
}
