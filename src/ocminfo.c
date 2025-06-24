/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma opt_code_size
#include <string.h>
#include "msx_const.h"
#include "conio.h"
#include "dos.h"
#include "globals.h"
#include "dialogs.h"
#include "profiles_ui.h"
#include "profiles_api.h"
#include "heap.h"
#include "utils.h"
#include "ocm_ioports.h"
#include "ocminfo.h"
#include "patterns.h"


// ========================================================
static uint8_t msxVersionROM;
static uint8_t kanjiMode;
static uint8_t originalLINL40;
static uint8_t originalSCRMOD;
static uint8_t originalFORCLR;
static uint8_t originalBAKCLR;
static uint8_t originalBDRCLR;
static bool isVisibleSetSmartText = false;
static uint8_t lastCmdSent = 0;
static uint16_t lastExtraKeys;
static uint16_t currentExtraKeys;
static bool end = false;
char *emptyArea;

OCM_P42_VirtualDIP_t virtualDIPs;
OCM_P43_LockToggles_t lockToggles;
OCM_P44_LedLights_t  ledLights;
OCM_P45_AudioVol0_t  audioVols0;
OCM_P46_AudioVol1_t  audioVols1;
OCM_P47_SysInfo0_t   sysInfo0;
OCM_P48_SysInfo1_t   sysInfo1;
OCM_P49_SysInfo2_t   sysInfo2;
OCM_P4A_SysInfo3_t   sysInfo3;
OCM_P4B_SysInfo4_0_t sysInfo4_0;
OCM_P4B_SysInfo4_1_t sysInfo4_1;
OCM_P4B_SysInfo4_2_t sysInfo4_2;
OCM_P4C_SysInfo5_t   sysInfo5;
OCM_P4E_Version0_t   pldVers0;
OCM_P4F_Version1_t   pldVers1;

uint8_t portsChecksum;

static Panel_t *currentPanel;
static Element_t *currentElement;
static Element_t *nextElement;
static Panel_t *nextPanel;


// ========================================================
// Function declarations

uint16_t commandLine(char **argv, int argc);
void setByteVRAM(uint16_t vram, uint8_t value) __sdcccall(0);
void _fillVRAM(uint16_t vram, uint16_t len, uint8_t value) __sdcccall(0);
void _copyRAMtoVRAM(uint16_t memory, uint16_t vram, uint16_t size) __sdcccall(0);

void abortRoutine();
void restoreScreen();
bool sendCommand(Element_t *elem);
static void drawCurrentPanel();
static bool drawElement(Element_t *element);


// ========================================================
static void checkPlatformSystem()
{
	#ifndef _DEBUG_
		// Check for OCM-PLD Device
		if (!ocm_detectDevice(DEVID_OCMPLD)) {
			die("OCM-PLD not detected!\n\r");
		}
	#endif

	// Check MSX2 ROM or higher
	msxVersionROM = getRomByte(MSXVER);
	if (!msxVersionROM) {
		die("MSX1 not supported!");
	}

	// Check MSX-DOS 2 or higher
	if (dosVersion() < VER_MSXDOS2x) {
		die("MSX-DOS 2.x or higher required!");
	}

	// Set abort exit routine
	dos2_setAbortRoutine((void*)abortRoutine);

	// Backup original values
	originalLINL40 = varLINL40;
	originalSCRMOD = varSCRMOD;
	originalFORCLR = varFORCLR;
	originalBAKCLR = varBAKCLR;
	originalBDRCLR = varBDRCLR;
}

char *play_fail = "\"v12l64o2b\"";
char *play_error = "\"v12l64o2br64b\"";
char *play_ok = "\"v12l64o4ar64o5c\"";
char *play_advice = "\"v12l64o4a\"";

static void beep_sound(char *sound)
{
	if (!profile_getHeaderData()->muteSound)
		basic_play(sound);
}

void beep_ok()
{
	beep_sound(play_ok);
}

void beep_advice()
{
	beep_sound(play_advice);
}

void beep_fail()
{
	beep_sound(play_fail);
}

void beep_error()
{
	beep_sound(play_error);
}

void abortRoutine()
{
	restoreScreen();
	dos2_exit(1);
}

void putstrxy(uint8_t x, uint8_t y, char *str)
{
	putlinexy(x, y, strlen(str), str);
}


// ========================================================
static uint8_t getOcmData()
{
	// Hardware ports values
	virtualDIPs.raw = ocm_getPortValue(OCM_VIRTDIPS_PORT);
	lockToggles.raw = ocm_getPortValue(OCM_LOCKTOGG_PORT);
	ledLights.raw = ocm_getPortValue(OCM_LEDLIGHT_PORT);
	audioVols0.raw = ocm_getPortValue(OCM_AUDVOLS0_PORT);
	audioVols1.raw = ocm_getPortValue(OCM_AUDVOLS1_PORT);
	sysInfo0.raw = ocm_getPortValue(OCM_SYSINFO0_PORT);
	sysInfo1.raw = ocm_getPortValue(OCM_SYSINFO1_PORT);
	sysInfo2.raw = ocm_getPortValue(OCM_SYSINFO2_PORT);
	sysInfo3.raw = ocm_getPortValue(OCM_SYSINFO3_PORT);
	sysInfo4_0.raw = ocm_getDynamicPortValue(0);
	sysInfo4_1.raw = ocm_getDynamicPortValue(1);
	sysInfo4_2.raw = ocm_getDynamicPortValue(2);
	sysInfo5.raw = ocm_getPortValue(OCM_SYSINFO5_PORT);
	pldVers0.raw = ocm_getPortValue(OCM_PLDVERS0_PORT);
	pldVers1.raw = ocm_getPortValue(OCM_PLDVERS1_PORT);

	// Custom virtual values
	customCpuClockValue = (!virtualDIPs.cpuClock ? 7 + sysInfo1.turboPana : sysInfo0.cpuCustomSpeed - 1 );
	customCpuModeValue = (!virtualDIPs.cpuClock ? sysInfo1.turboPana : 2 );
	customVideoOutputValue = customVideoOutputMap[virtualDIPs.videoOutput_raw];
	customSlots12Value = customSlots12Map[(virtualDIPs.raw >> 3) & 0b111];
	customLockAllToggles = lockToggles.raw == 255 ? 1 : 0;

	// Additional customs
	// - Vertical Offset status only exists for I/O rev >= 12
	if (pldVers1.ioRevision > IOREV_11) {
		customVerticalOffsetValue = sysInfo4_1.verticalOffset - 4;
	}
	// - OCM bug in VideoMode when VGA 1:1 is enabled for I/O rev <= 11
	if (customVideoOutputValue != VIDEOUTPUT_VGA11 || pldVers1.ioRevision > IOREV_11) {
		customVideoModeValue = (sysInfo2.videoType ? 1 : (sysInfo2.videoForcedMode ? 0 : 2));
	}

	// Checksum
	portsChecksum = virtualDIPs.raw ^ lockToggles.raw ^ ledLights.raw ^ audioVols0.raw ^ 
					audioVols1.raw ^ sysInfo0.raw ^ sysInfo1.raw ^ sysInfo2.raw ^ sysInfo3.raw ^ 
					sysInfo4_0.raw ^ sysInfo4_1.raw ^ sysInfo4_2.raw ^ sysInfo5.raw ^ 
					pldVers0.raw ^ pldVers1.raw;
	return portsChecksum;
}

// ========================================================
static void printHeader()
{
	char *sdram = getString(
		pldVers1.ioRevision < IOREV_11 ?
			sdramSizeStr[3] :
			sysInfo4_0.sdramSize != 3 ? sdramSizeStr[sysInfo4_0.sdramSize] : sdramSizeAuxStr[sysInfo4_1.sdramSizeAux]
	);

	textblink(1,1, 80, true);

	csprintf(heap_top, getString(HEADER_MODEL_SDRAM),
		getString(machineTypeStr[pldVers1.ioRevision < IOREV_4 ? MACHINETYPE_UNKNOWN : sysInfo2.machineTypeId]), 
		sdram);
	putstrxy(3,1, heap_top);
	if (pldVers1.ioRevision < IOREV_5) {
		csprintf(heap_top, getString(HEADER_PLD_LEGACY),
			pldVers1.ioRevision);
	} else {
		csprintf(heap_top, getString(HEADER_PLD_CURRENT),
			pldVers0.pldVersion / 10, 
			pldVers0.pldVersion % 10, 
			pldVers1.pldSubversion, 
			pldVers1.ioRevision);
	}
	putstrxy(79-strlen(heap_top),1, heap_top);

	// Function keys topbar
	drawFrame(1,2, 80,24);
	Panel_t *panel = &pPanels[PANEL_FIRST];
	while (panel->title != ARRAYEND) {
		putlinexy(panel->titlex,panel->titley, panel->titlelen, getString(panel->title));
		panel++;
	}

	// Elements panel
	chlinexy(2,4, 78);

	// Info panel
	chlinexy(2,20, 78);

	// Version
	csprintf(heap_top, getString(HEADER_NAME), getString(HEADER_VERSION));
	uint16_t verLen = strlen(heap_top);
	putlinexy(78-verLen, 24, verLen, heap_top);
}

static void drawDescription(uint16_t *description)
{
	waitVBLANK();

	// Clear Description zone
	puttext(2,21, 79,23, emptyArea);

	// Print new Description
	for (uint8_t i = 0; i < ELEMENT_MAX_DESC ; i++) {
		if (description[i] == ARRAYEND) break;
		putstrxy(3,21+i, getString(description[i]));
	}
}

static void drawSetSmartText()
{
	if (lastCmdSent != OCM_SMART_NullCommand) {
		csprintf(heap_top, getString(INFO_SETSMART_CMD), lastCmdSent/16, lastCmdSent%16);
		putlinexy(SETSMART_X,SETSMART_Y, SETSMART_SIZE, heap_top);
		isVisibleSetSmartText = true;
	}
}


// ========================================================
bool isIOrevisionSupported(Element_t *element)
{
	return pldVers1.ioRevision >= element->ioRevNeeded;
}

bool isMachineSupported(Element_t *element)
{
	return ((1<<sysInfo2.machineTypeId) & element->supportedBy) != 0;
}


// ========================================================
static uint8_t getValueOffset(uint8_t mask)
{
	uint8_t offset = 0;
	if (mask) {
		while ((mask & 1) == 0) {
			offset++;
			mask >>= 1;
		}
	}
	return offset;
}

static void setValue(Element_t *elem, uint8_t value)
{
	value <<= getValueOffset(elem->valueMask);
	value &= elem->valueMask;

	*elem->value &= ~(elem->valueMask);
	*elem->value |= value;
}

static uint8_t getValue(Element_t *elem)
{
	return (*(elem->value) & elem->valueMask) >> getValueOffset(elem->valueMask);
}

static bool changeCurrentValue(int8_t increase)
{
	if (!isIOrevisionSupported(currentElement) || 
		!isMachineSupported(currentElement) ||
		currentElement->cmdType == CMDTYPE_NONE ||
		currentElement->value == NULL)
	{
		return false;
	}

	int8_t value = getValue(currentElement);
	value += increase;
	if (value < currentElement->minValue) value = currentElement->maxValue;
	if (value > currentElement->maxValue) value = currentElement->minValue;
	if (currentElement->cmdType != CMDTYPE_NONE) {		// if is RW
		setValue(currentElement, value);
		
		if (sendCommand(currentElement)) {
			// Display setsmart text
			drawSetSmartText();
		} else {
			// Element disabled
			currentElement->supportedBy = M_NONE;
			return false;
		}
		return true;
	}
	return false;
}

// ========================================================
uint8_t getActiveCommand(Element_t *elem)
{
	uint8_t elemCmd = elem->cmd[getValue(elem)];

	// Single Standard Command
	if (elem->cmdType == CMDTYPE_STANDARD) {
		return elemCmd;
	} else
	// Custom Command behaviours
	if (elem->cmdType == CMDTYPE_CUSTOM_CPUMODE) {
		// If custom cpu speed then set the current custom speed directly
		if (elemCmd == OCM_SMART_NullCommand) {
			return elemSystem[CUSTOM_SPEED_IDX].cmd[sysInfo0.cpuCustomSpeed];
		}
		return elemCmd;
	} else
	if (elem->cmdType == CMDTYPE_CUSTOM_SLOTS12) {
		return elem->cmd[customSlots12Value];
	}

	return OCM_SMART_NullCommand;
}

bool sendCommand(Element_t *elem)
{
	uint8_t cmd = getActiveCommand(elem);
	lastCmdSent = cmd;

	// Send Command
	bool result = ocm_sendSmartCmd(cmd);
	getOcmData();
	if (result) {
		if (cmd == OCM_SMART_ResetDefaults) {
			customAudioPresetValue = 0;
		}
	}
	return result;
}

void getPanelsCmds(uint8_t *cmd)
{
	#define LASTCMDS_SIZE 40

	Panel_t *panel = &pPanels[PANEL_FIRST];
	Element_t *element;
	uint8_t *lastCmds = malloc(LASTCMDS_SIZE), *lastPtr;
	uint8_t *ptr = cmd, cmdToAdd;
	uint8_t cmdCount = 0;

	*cmd = 0x00;
	while (panel->title != ARRAYEND) {
		element = panel->elements;
		panel++;
		if (element == NULL) continue;
		lastPtr = lastCmds;
		*lastCmds = 0x00;
		while (element->type != END) {
			if (element->saveToProfile && 
				isMachineSupported(element) && 
				isIOrevisionSupported(element))
			{
				cmdToAdd = getActiveCommand(element);
				if (cmdToAdd != OCM_SMART_NullCommand) {
					if (element->sendSmartLast) {
						// Add command to lastCmds (check bounds)
						if (lastPtr - lastCmds < LASTCMDS_SIZE - 1) {
							*lastPtr++ = cmdToAdd;
							*lastPtr = 0x00;
						}
					} else {
						// Add command to cmd (check bounds)
						ptr = cmd;
						while (*ptr && *ptr != cmdToAdd) ptr++;
						if (!*ptr && cmdCount < PROF_CMDSIZE - 1) {
							*ptr++ = cmdToAdd;
							*ptr = 0x00;
							cmdCount++;
						}
					}
				}
			}
			element++;
		}
		// Dump elements with sendSmartLast enabled
		if (*lastCmds) {
			lastPtr = lastCmds;
			while (*lastPtr && cmdCount < PROF_CMDSIZE - 1) {
				ptr = cmd;
				while (*ptr && *ptr != *lastPtr) ptr++;
				if (!*ptr) {
					*ptr++ = *lastPtr;
					*ptr = 0x00;
					cmdCount++;
				}
				lastPtr++;
			}
		}
	}
	free(LASTCMDS_SIZE);
}


// ========================================================
static void drawWidget_slider(Element_t *element)
{
	uint8_t posx = wherex();
	char sliderStr[] = "\x81\x81\x81\x81\x81\x81\x81\x81\x81";
	uint8_t value = getValue(element);

	sliderStr[element->maxValue - element->minValue + 1] = '\0';
	sliderStr[value - element->minValue] = '\x83';
	if (element->valueStr != NULL) {
		csprintf(heap_top, "-\x80%s\x82+  %s", sliderStr, getString(element->valueStr[value]));
	} else {
		csprintf(heap_top, "-\x80%s\x82+  %u  ", sliderStr, value);
	}
	putstrxy(posx, wherey(), heap_top);
}

static void drawWidget_value(Element_t *element)
{
	uint8_t value = getValue(element);
	if (element->valueStr == NULL) {
		csprintf(heap_top, "%u", value);
	} else {
		csprintf(heap_top, "%s", getString(element->valueStr[value]));
	}
	putstrxy(wherex() + element->maxValue, wherey(), heap_top);
}

static void drawCustom_cpuSpeed(Element_t *element)
{
	Element_t *elemChange = &currentPanel->elements[3];
	if (!virtualDIPs.cpuClock) {
		// Standard / TurboPana speed
		elemChange->supportedBy = M_NONE;		// Element 'Custom speed' disabled
		putlinexy(elemChange->posX + strlen(getString(elemChange->label)), elemChange->posY, 12, emptyArea);
	} else {
		// Custom speed
		elemChange->supportedBy = M_ALL;		// Element 'Custom speed' enabled
	}
	drawWidget_value(element);
}

static void drawCustom_volume(Element_t *element)
{
	drawWidget_slider(element);
	if (currentElement == element) {
		customAudioPresetValue = 0;
		drawElement(&elemAudio[0]);
	}
}

static bool drawElement(Element_t *element)
{
	if (element->type == END) return false;

	uint8_t posx = element->posX;
	uint8_t posy = element->posY;

	putstrxy(posx, posy, getString(element->label));

	if (element->type == LABEL) return true;

	posx += element->valueOffsetX;
	if (!isIOrevisionSupported(element) || !isMachineSupported(element)) {
		char *text = getString(element->useLastStrForNA ? element->valueStr[element->maxValue+1] : LABEL_NA);
		putlinexy(posx, posy, element->maxValue + 6, emptyArea);
		putstrxy(
			posx + element->maxValue + 7,
			posy,
			text
		);
		return true;
	}

	posx += element->minValue;
	gotoxy(posx, posy);

	switch (element->type) {
		case LABEL:
		case BUTTON:
			break;
		case VALUE:
			drawWidget_value(element);
			break;
		case SLIDER:
			drawWidget_slider(element);
			break;
		case CUSTOM_CPUCLOCK_VALUE:
			drawCustom_cpuSpeed(element);
			break;
		case CUSTOM_VOLUME_SLIDER:
			drawCustom_volume(element);
			break;
	}
	return true;
}

static void selectCurrentElement(bool enable)
{
	textblink(
		currentElement->posX, currentElement->posY,
		strlen(getString(currentElement->label)),
		enable);
	if (enable) {
		drawDescription(currentElement->description);
	}
}

static void pressedCurrentElement(uint8_t increment)
{
	bool changeResult = false;
	bool commandToSend = true;

	if (currentElement->areYouSure) {
		commandToSend = (showDialog(&dlg_confirm) == BTN_YES);
		selectCurrentElement(true);
	}

	if (commandToSend) {
		if (currentElement->type == BUTTON) {
			changeResult = sendCommand(currentElement);
			drawSetSmartText();
		} else {
			changeResult = changeCurrentValue(increment);
			drawElement(currentElement);
		}
	}

	if (changeResult) {
		beep_advice();

		// Force Panel Reload management
		if (currentElement->forcePanelReload) {
			drawCurrentPanel();
		}

		// Requested Reset management
		if (sysInfo1.resetReqFlag && currentElement->needResetToApply) {
			if (showDialog(&dlg_reset) == 0) {
				ocm_sendSmartCmd(sysInfo1.lastResetFlag ? OCM_SMART_WarmReset : OCM_SMART_ColdReset);
			}
		}
	} else {
		beep_fail();
	}
}

// ========================================================
static void drawCurrentPanel()
{
	Element_t *element = &(currentPanel->elements[0]);
	while (drawElement(element)) {
		element++;
	}
}

static void selectPanelTitle(Panel_t *panel)
{
	if (currentPanel != NULL) {
		textblink(1, panel->titley, 80, false);
		selectCurrentElement(false);
	}

	// Set title blink
	textblink(panel->titlex, panel->titley, panel->titlelen, true);
}

static void selectPanel(Panel_t *panel)
{
	// Refresh I/O ext values
	getOcmData();

	waitVBLANK();

	// Update blinks
	selectPanelTitle(panel);

	// Clear Panel zone
	if (currentPanel != panel) {
		puttext(2,5, 79,19, emptyArea);
	}

	// Draw Panel elements
	currentPanel = panel;
	drawCurrentPanel();

	// Select first element as current
	currentElement = nextElement = &currentPanel->elements[0];
	while (currentElement->type != END && currentElement->type == LABEL) {
		currentElement++;
	}
	if (currentElement->type == END) {
		currentElement = nextElement;
	} else {
		nextElement = currentElement;
	}

	// Set first element selected
	selectCurrentElement(true);
}

inline void redefineFunctionKeys()
{
	char *fk = (char*)FNKSTR;
	memset(fk, 0, 160);
	for (uint8_t i='1'; i<='5'; i++,fk+=16) {
		*fk = i;
	}
}

inline void redefineCharPatterns()
{
	_copyRAMtoVRAM((uint16_t)charPatters, 0x1000+0x7f*8, 5*8);
}

inline bool isShiftKeyPressed()
{
	return varNEWKEY_row6.shift == 0;
}

// ========================================================
void menu_panels()
{
	kanjiMode = (detectKanjiDriver() ? getKanjiMode() : 0);
	if (kanjiMode) {
		setKanjiMode(0);
	}

	// Initialize empty panel
	emptyArea = malloc(78*21);
	memset(emptyArea, ' ', 78*21);

	//Platform system checks
	checkPlatformSystem();

	// Load profile file
	profile_loadFile();

	// Initialize screen 0[80]
	textmode(BW80);
	textattr(0xa1f4);
	setcursortype(NOCURSOR);
	redefineFunctionKeys();
	redefineCharPatterns();

	// Get data from I/O extension ports
	getOcmData();

	// Initialize header & panel
	printHeader();
	currentPanel = NULL;
	selectPanel(&pPanels[PANEL_FIRST]);

	// Main loop panels
	lastExtraKeys = getExtraKeysOCM().raw;
	currentExtraKeys = lastExtraKeys;
	do {
		while (!kbhit() && lastExtraKeys == currentExtraKeys) {
			waitVBLANK();
			currentExtraKeys = getExtraKeysOCM().raw;
		}

		// Clear last setsmart text
		if (isVisibleSetSmartText || lastExtraKeys != currentExtraKeys) {
			putlinexy(SETSMART_X,SETSMART_Y, SETSMART_SIZE, emptyArea);
			isVisibleSetSmartText = false;
		}

		// If OCM extra key pressed/realeased the panel is updated
		if (lastExtraKeys != currentExtraKeys) {
			uint8_t lastPortsChecksum = portsChecksum;
			if (lastPortsChecksum != getOcmData()) {
				beep_advice();
			} else {
				beep_fail();
			}
			while (lastExtraKeys != currentExtraKeys) {
				waitVBLANK();
				currentExtraKeys = getExtraKeysOCM().raw;
				getOcmData();
				drawCurrentPanel();
			}
			continue;
		}

		// Manage pressed key
		switch(dos2_toupper(getch())) {
			case KEY_UP:
				nextElement = currentElement + currentElement->goUp;
				break;
			case KEY_DOWN:
				nextElement = currentElement + currentElement->goDown;
				break;
			case KEY_LEFT:
				nextElement = currentElement + currentElement->goLeft;
				break;
			case KEY_RIGHT:
				nextElement = currentElement + currentElement->goRight;
				break;
			case KEY_SPACE:
			case KEY_ENTER:
			case '+':
				pressedCurrentElement(1);
				break;
			case KEY_BS:
			case '-':
				pressedCurrentElement(-1);
				break;
			case '1':
				selectPanel(&pPanels[PANEL_SYSTEM]);
				break;
			case '2':
				selectPanel(&pPanels[PANEL_VIDEO]);
				break;
			case '3':
				selectPanel(&pPanels[PANEL_AUDIO]);
				break;
			case '4':
				selectPanel(&pPanels[PANEL_DIPS]);
				break;
			case '5':
				selectPanel(&pPanels[PANEL_LOCKS]);
				break;
			case 'A':
				selectPanel(&pPanels[PANEL_HELP]);
				break;
			case KEY_TAB:
				nextPanel = currentPanel;
				if (!isShiftKeyPressed()) {
					nextPanel++;
					if (nextPanel > &pPanels[PANEL_LAST]) {
						nextPanel = &pPanels[PANEL_FIRST];
					}
				} else {
					nextPanel--;
					if (nextPanel < &pPanels[PANEL_FIRST]) {
						nextPanel = &pPanels[PANEL_LAST];
					}
				}
				selectPanel(nextPanel);
				break;
			case 'P':
				selectCurrentElement(false);
				profiles_menu(&pPanels[PANEL_PROFILES]);
				printHeader();
				selectPanel(currentPanel);
				break;
			case 'X':
			case KEY_ESC:
				selectCurrentElement(false);
				selectPanelTitle(&pPanels[PANEL_EXIT]);
				if (showDialog(&dlg_exit) == 0) {
					end++;
				}
				selectPanelTitle(currentPanel);
				selectCurrentElement(true);
				break;
		}
		if (currentElement != nextElement) {
			selectCurrentElement(false);
			currentElement = nextElement;
			selectCurrentElement(true);
		}
		varPUTPNT = varGETPNT;
		varREPCNT = 0;
	} while (!end);

	restoreScreen();
}

void restoreOriginalScreenMode() __naked
{
	// Restore original screen mode
	__asm
		ld   a, (_originalSCRMOD)
		or   a
		jr   nz, .screen1
		ld   ix, #INITXT				; Restore SCREEN 0
		jr   .bioscall
	.screen1:
		ld   ix, #INIT32				; Restore SCREEN 1
	.bioscall:
		JP_BIOSCALL
	__endasm;
}

void restoreScreen()
{
	// Clear & restore original screen parameters & colors
	__asm
		ld   ix, #DISSCR				; Disable screen
		BIOSCALL
		ld   ix, #INIFNK				; Restore function keys
		BIOSCALL
	__endasm;

	textattr(0x00f4);				// Clear blink
	_fillVRAM(0x0800, 240, 0);

	varLINL40 = originalLINL40;
	varFORCLR = originalFORCLR;
	varBAKCLR = originalBAKCLR;
	varBDRCLR = originalBDRCLR;

	if (kanjiMode) {
		// Restore kanji mode if needed
		setKanjiMode(kanjiMode);
	} else {
		// Restore original screen mode
		restoreOriginalScreenMode();
	}

	__asm
		ld   ix, #ENASCR
		BIOSCALL
	__endasm;

	// Restore abort routine
	dos2_setAbortRoutine(0x0000);
}

// ========================================================
int main(char **argv, int argc) __sdcccall(0)
{
	// A way to avoid using low memory when using BIOS calls from DOS
	if (heap_top < (void*)0x8000)
		heap_top = (void*)0x8000;

	// Initialize compressed strings
	stringsInit();

	if (argc != 0) {
		return commandLine(argv, argc);
	}
	menu_panels();
	return 0;
}
