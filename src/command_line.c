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
#include "globals.h"
#include "ocm_ioports.h"
#include "profiles_api.h"


// ========================================================
static ProfileItem_t *item;
static bool verbose = true;


// ========================================================
void printHelp()
{
	cputs(
		"https://github.com/nataliapc/msx_ocminfo\n"
		"\n"
		"Usage: OCMINFO [/n|/L] [/Q] [/?]\n"
		"\n"
		"Use without parameters to open the configuration panels.\n"
		"\n"
		"Options:\n"
		"  /n    Apply the user profile 'n'.\n"
		"  /L    List the user profiles.\n"
		"  /R    Reset OCM to default values.\n"
		"  /Q    Quiet mode (no verbose).\n"
		"  /?    Show this help.\n"
	);
}

bool readProfilesFile()
{
	if (profile_loadFile()) {
		return true;
	}
	cputs("ERROR: No profiles file to read!\n");
	return false;
}

void doResetToDefaults()
{
	if (verbose) cputs("Reset OCM to default values.\n"
					   "setsmart -FF\n\n");
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
					cputs("[Press a key to continue]");
					getch();
					cputs("\r"VT_CLREOL);
				}
			}
		} else {
			cputs("ERROR: No profiles to list!\n");
		}
	}
}

void doApplyProfile(uint8_t idx)
{
	if (profile_loadFile()) {
		if (item = profile_getItem(idx-1)) {
			uint8_t *cmd = item->cmd;
			if (verbose) {
				cprintf("Applying Profile #%u: \"%s\"\n"
						"setsmart -", idx, item->description);
			}
			while (*cmd) {
				ocm_sendSmartCmd(*cmd);
				if (verbose) {
					if (*cmd < 16) putch('0');
					cprintf("%x", *cmd);
				}
				cmd++;
			}
		} else {
			cprintf("ERROR: Profile #%u not found!", idx);
		}
	}
}

uint16_t commandLine(char **argv, int argc)
{
	bool showHelp = true;
	uint16_t profileToApply = 0;
	bool listProfiles = false;
	bool paramDetected = false;
	bool resetDetected = false;
	uint8_t i = 0;
	char *arg;

	while(i < argc) {
		arg = argv[i++];
		arg[1] = dos2_toupper(arg[1]);
		if (*arg++ != '/') {
			showHelp = true;
			break;
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
					cputs("ERROR: Invalid profile index!\n\n");
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

	if (showHelp || verbose) {
		cputs("OCMINFO "VERSION" by "AUTHOR"\n");
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
			doApplyProfile((uint8_t)profileToApply);
		}
	}

	return 1;
}
