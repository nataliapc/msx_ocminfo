/*
	Copyright (c) 2022 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <string.h>
#include "conio.h"
#include "dialogs.h"
#include "heap.h"
#include "msx_const.h"
#include "utils.h"
#include "ocm_ioports.h"

#pragma opt_code_size


// ========================================================
static uint8_t msxVersionROM;
static uint8_t kanjiMode;
static char *emptyArea;
static bool isVisibleSetSmartText = false;

static OCM_P42_VirtualDIP_t virtualDIPs;
static OCM_P43_LockToggles_t lockToggles;
static OCM_P44_LedLights_t ledLights;
static OCM_P45_AudioVol0_t audioVols0;
static OCM_P46_AudioVol1_t audioVols1;
static OCM_P47_SysInfo0_t sysInfo0;
static OCM_P48_SysInfo1_t sysInfo1;
static OCM_P49_SysInfo2_t sysInfo2;
static OCM_P4A_SysInfo3_t sysInfo3;
static OCM_P4B_SysInfo4_t sysInfo4;
static OCM_P4C_SysInfo5_t sysInfo5;
static OCM_P4E_Version0_t pldVers0;
static OCM_P4F_Version1_t pldVers1;

static Panel_t *currentPanel;
static Element_t *currentElement;
static Element_t *nextElement;


// ========================================================
static void drawCurrentPanel();


// ========================================================
static void checkPlatformSystem()
{
	msxVersionROM = getRomByte(MSXVER);

	// Check for OCM-PLD Device
	if (!ocm_detectDevice(DEVID_OCMPLD)) {
		die("ERROR: OCM-PLD not detected!\n\r");
	}

	if (!msxVersionROM) {
		die("This don't works on MSX1!");
	}
}

// ========================================================
static void getOcmData()
{
	virtualDIPs.raw = ocm_getPortValue(OCM_VIRTDIPS_PORT);
	lockToggles.raw = ocm_getPortValue(OCM_LOCKTOGG_PORT);
	ledLights.raw = ocm_getPortValue(OCM_LEDLIGHT_PORT);
	audioVols0.raw = ocm_getPortValue(OCM_AUDVOLS0_PORT);
	audioVols1.raw = ocm_getPortValue(OCM_AUDVOLS1_PORT);
	sysInfo0.raw = ocm_getPortValue(OCM_SYSINFO0_PORT);
	sysInfo1.raw = ocm_getPortValue(OCM_SYSINFO1_PORT);
	sysInfo2.raw = ocm_getPortValue(OCM_SYSINFO2_PORT);
	sysInfo3.raw = ocm_getPortValue(OCM_SYSINFO3_PORT);
	sysInfo4.raw = ocm_getPortValue(OCM_SYSINFO4_PORT);
	sysInfo5.raw = ocm_getPortValue(OCM_SYSINFO5_PORT);
	pldVers0.raw = ocm_getPortValue(OCM_PLDVERS0_PORT);
	pldVers1.raw = ocm_getPortValue(OCM_PLDVERS1_PORT);

	customCpuSpeedValue = (!virtualDIPs.cpuClock ? 7 + sysInfo1.turboPana : sysInfo0.cpuCustomSpeed - 1 );
	customCpuModeValue = (!virtualDIPs.cpuClock ? sysInfo1.turboPana : 2 );
}

// ========================================================
static void printHeader()
{
	textblink(1,1, 80, true);

	csprintf(heap_top, "Model: %s     SDRAM: %sMb     PLD v%u.%u.%u     I/O rev.%u",
		machineTypeStr[sysInfo2.machineTypeId], 
		sdramSizeStr[sysInfo4.sdramSize], 
		pldVers0.pldVersion / 10, 
		pldVers0.pldVersion % 10, 
		pldVers1.pldSubversion, 
		pldVers1.ioRevision);
	putlinexy(5,1, strlen(heap_top), heap_top);

	// Function keys panel
	drawFrame(1,2, 80,24);
	Panel_t *panel = pPanels;
	while (panel->title != NULL) {
		putlinexy(panel->titlex,panel->titley, panel->titlelen, panel->title);
		panel++;
	}

	// Elements panel
	chlinexy(2,4, 78);
	emptyArea = malloc(78*15);
	memset(emptyArea, ' ', 78*15);

	// Info panel
	chlinexy(2,20, 78);

	// Version
	uint16_t verLen = strlen(ocminfoVersionStr);
	putlinexy(78-verLen, 24, verLen, ocminfoVersionStr);

}

// ========================================================
inline bool isIOrevisionSupported(Element_t *element)
{
	return pldVers1.ioRevision >= element->ioRevNeeded;
}

inline bool isMachineSupported(Element_t *element)
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

static uint8_t getMaxValue(Element_t *elem)
{
	return elem->valueMask >> getValueOffset(elem->valueMask);
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
		currentElement->cmd[0] == 0x00 ||
		currentElement->value == NULL)
	{
		putch('\x07');
		return false;
	}

	uint8_t maxValue = getMaxValue(currentElement);
	int8_t value = getValue(currentElement);
	value += increase;
	if (value < currentElement->minValue) value = currentElement->maxValue;
	if (value > currentElement->maxValue) value = currentElement->minValue;
	value &= maxValue;
	if (currentElement->cmd[0] != 0x00) {		// if is RW
		ocm_sendSmartCmd(currentElement->cmd[value]);
		if (currentElement->forcePanelReload) {
			getOcmData();
			drawCurrentPanel();
			value = getValue(currentElement);
		} else {
			setValue(currentElement, value);
		}

		// Display setsmart text
		char digit0[] = "0";
		if (currentElement->cmd[value] >= 16) *digit0 = '\0';
		csprintf(heap_top, "setsmart -%s%x", digit0, currentElement->cmd[value]);
		putlinexy(SETSMART_X,SETSMART_Y, strlen(heap_top), heap_top);
		isVisibleSetSmartText = true;
		
		return true;
	}
	return false;
}

// ========================================================
static void drawDescription(char **description)
{
	// Clear Description zone
	puttext(2,21, 79,23, emptyArea);

	// Print new Description
	uint8_t i = 0;
	while (description[i] != NULL) {
		putlinexy(3,21+i, strlen(description[i]), description[i]);
		i++;
	}
}

// ========================================================
static void drawWidget_slider(Element_t *element)
{
	uint8_t posx = wherex();
	char sliderStr[] = "========";
	uint8_t value = getValue(element);

	sliderStr[element->maxValue - element->minValue + 1] = '\0';
	if (element->valueStr != NULL) {
		csprintf(heap_top, "-[%s]+  %s", sliderStr, element->valueStr[value]);
	} else {
		csprintf(heap_top, "-[%s]+  %u  ", sliderStr, value);
	}
	*(heap_top+2+value-element->minValue) = '\x85';
	putlinexy(posx, wherey(), strlen(heap_top), heap_top);
}

static void drawWidget_value(Element_t *element)
{
	uint8_t value = getValue(element);
	if (element->valueStr == NULL) {
		csprintf(heap_top, "%u", value);
	} else {
		csprintf(heap_top, "%s", element->valueStr[value]);
	}
	putlinexy(wherex(), wherey(), strlen(heap_top), heap_top);
}

static void drawCustom_cpuSpeed(Element_t *element)
{
	Element_t *elemChange = &currentPanel->elements[3];
	if (!virtualDIPs.cpuClock) {
		// Standard / TurboPana speed
		elemChange->supportedBy = M_NONE;		// Element 'Custom speed'
		elemChange->valueOffsetX = 24;
		putlinexy(elemChange->posX + strlen(elemChange->label), elemChange->posY, 12, emptyArea);
	} else {
		// Custom speed
		elemChange->supportedBy = M_ALL;		// Element 'Custom speed'
		elemChange->valueOffsetX = 14;
	}
	drawWidget_value(element);
}

static bool drawElement(Element_t *element)
{
	if (element->type == END) return false;

	uint8_t posx = element->posX;
	uint8_t posy = element->posY;

	putlinexy(posx, posy, strlen(element->label), element->label);

	if (element->type == LABEL) return true;

	if (!isIOrevisionSupported(element) || !isMachineSupported(element)) {
		putlinexy(posx+element->valueOffsetX+4, posy, 7, "n/a    ");
		return true;
	}

	posx += element->valueOffsetX + element->minValue;
	gotoxy(posx, posy);

	switch (element->type) {
		case LABEL:
			break;
		case VALUE:
			drawWidget_value(element);
			break;
		case SLIDER:
			drawWidget_slider(element);
			break;
		case CUSTOM_CPUSPEED:
			drawCustom_cpuSpeed(element);
			break;
	}
	return true;
}

static void selectCurrentElement(bool enable)
{
	textblink(
		currentElement->posX, currentElement->posY, 
		strlen(currentElement->label), 
		enable);
	if (enable) {
		drawDescription(currentElement->description);
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

// ========================================================
static void selectPanel(Panel_t *panel)
{
	ASM_EI;
	ASM_HALT;

	// Clear blinks
	if (currentPanel != NULL) {
		textblink(currentPanel->titlex, currentPanel->titley, currentPanel->titlelen, false);
		textblink(currentElement->posX, currentElement->posY, strlen(currentElement->label), false);
	}

	// Clear Panel zone
	puttext(2,5, 79,19, emptyArea);

	// Set title blink
	textblink(panel->titlex, panel->titley, panel->titlelen, true);

	// Refresh I/O ext values
	getOcmData();

	// Draw Panel elements
	currentPanel = panel;
	drawCurrentPanel();

	// Select first element as current
	currentElement = nextElement = &(currentPanel->elements[0]);
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

// ========================================================
void main(void)
{
	Panel_t *nextPanel;
	
	kanjiMode = (detectKanjiDriver() ? getKanjiMode() : 0);
	if (kanjiMode) {
		setKanjiMode(0);
	}

	// A way to avoid using low memory parameters for BIOS calls.
	if (heap_top < (void*)0x8000)
		heap_top = (void*)0x8000;

	//Platform system checks
	checkPlatformSystem();

	// Initialize screen 0[80]
	textmode(BW80);
	textattr(0xa1f4);
	setcursortype(NOCURSOR);
	redefineFunctionKeys();
	// Redefine '=' pattern character
	setByteVRAM(0x1000+61*8+2, 255);
	setByteVRAM(0x1000+61*8+4, 255);

	// Get data from I/O extension ports
	getOcmData();

	// Initialize header & panel
	printHeader();
	currentPanel = NULL;
	selectPanel(&pPanels[PANEL_SYSTEM]);

	bool end = false;
	do {
		if (kbhit()) {
			// Clear last setsmart text
			if (isVisibleSetSmartText) {
				putlinexy(SETSMART_X,SETSMART_Y, 12, "            ");
				isVisibleSetSmartText = false;
			}
			// Manage pressed key
			switch(getch()) {
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
					if (changeCurrentValue(1)) {
						drawElement(currentElement);
					}
					break;
				case '-':
					if (changeCurrentValue(-1)) {
						drawElement(currentElement);
					}
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
				case '5':
					selectPanel(&pPanels[PANEL_DIPS]);
					break;
				case KEY_TAB:
					nextPanel = currentPanel + 1;
					if (nextPanel->elements == NULL) {
						nextPanel = &pPanels[0];
					}
					selectPanel(nextPanel);
					break;
				case 'h':
				case 'H':
					selectPanel(&pPanels[PANEL_HELP]);
					break;
				case 'q':
				case 'Q':
				case KEY_ESC:
					end++;
					break;
			}
			if (currentElement != nextElement) {
				selectCurrentElement(false);
				currentElement = nextElement;
				selectCurrentElement(true);
			}
		}
		varPUTPNT = varGETPNT;
		varREPCNT = 0;
		ASM_EI;
		ASM_HALT;
	} while (!end);

	// Clean & restore screen
	textattr(0x00f4);
	_fillVRAM(0x1b00, 240, 0);
	clrscr();
	if (kanjiMode) {
		setKanjiMode(kanjiMode);
	} else {
		__asm
			push ix
			ld ix, #INITXT
			BIOSCALL
			ld ix, #INIFNK
			BIOSCALL
			pop ix
		__endasm;
	}
	exit();
}
