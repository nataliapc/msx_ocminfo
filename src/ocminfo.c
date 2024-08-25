/*
	Copyright (c) 2022 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <string.h>
#include "conio.h"
#include "heap.h"
#include "msx_const.h"
#include "utils.h"
#include "ocm_ioports.h"

#pragma opt_code_size

#define VERSION		"0.9.1"

typedef enum {
	LABEL,									// No widget, just label
	VALUE,									// Show numeric value
	ONOFF,									// Show OFF/ON pair
	SLIDER,									// Slider with ranges of 0 to 2/4/8 values
	END										// **End of Elements array
} Widget_t;

typedef enum {
	IOREV_ALL = 0,							// Supported by all revisions
	IOREV_1,								// OCM-PLD Pack v2.4
	IOREV_2,								// OCM-PLD Pack v3.0
	IOREV_3,								// OCM-PLD Pack v3.1
	IOREV_4,								// OCM-PLD Pack v3.2
	IOREV_5,								// OCM-PLD Pack v3.4
	IOREV_6,								// OCM-PLD Pack v3.5
	IOREV_7,								// OCM-PLD Pack v3.6
	IOREV_8,								// OCM-PLD Pack v3.6.2
	IOREV_9,								// OCM-PLD Pack v3.7
	IOREV_10,								// OCM-PLD Pack v3.9
	IOREV_11,								// OCM-PLD Pack v3.9.1
	IOREV_NA = 0xff							// Not/Available
} IOrev_t;

typedef enum {
	M_NONE            = 0x0000,				// None machine supported
	M_OCM_ID          = 0x0001,				// Device 0 (1<<0)
	M_ZEMNEO_SX1_ID   = 0x0002,				// Device 1 (1<<1)
	M_SMX_MCP2_ID     = 0x0004,				// Device 2 (1<<2)
	M_SX2             = 0x0008,				// Device 3 (1<<3)
	M_SMXMIN_SMXHB_ID = 0x0010,				// Device 4 (1<<4)
	M_DE0CV_ID        = 0x0020,				// Device 5 (1<<5)
	M_ALL             = 0xffff				// All devices supported
} MachineMask_t;

typedef struct {
	Widget_t type;							// Widget type
	uint8_t x, y;							// Element position
	char *label;							// Element text label
	int8_t goUp, goDown, goLeft, goRight;	// Navigation to indexes (relative offset)
	uint8_t *value;							// Cached full byte for value
	uint8_t valueMask;						// Mask for value
	uint8_t **valueStr;						// Array with labels for each value
	uint8_t valueOffsetX;					// Offset X for value widget
	uint8_t cmd[8];							// OCM Smart Cmd to set each value
	bool forcePanelReload;					// Force panel reload when value changes
	char *description[4];					// Description lines
	IOrev_t ioRevNeeded;					// I/O Revision needed [0x00:all 0xff:n/a]
	MachineMask_t supportedBy;				// Supported machines
} Element_t;

typedef struct {
	char *title;							// Panel name at top header
	uint8_t titlex, titley, titlelen;		// Position & llength at top header
	Element_t *elements;					// Array of elements
} Panel_t;


// ========================================================
static uint8_t msxVersionROM;
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
static const char *ocminfoVersionStr = "\x13 ocminfo v"VERSION" \x14";

static const char *machineTypeStr[16] = {
	"1ChipMSX", "Zemmix Neo/SX-1", "SM-X/MCP2", "SX-2", "SM-X Mini/SM-X HB", 
	"DE0CV", "??", "??", "??", "??", "??", "??", "??", "??", "??", "Unknown"
};
static const char *sdramSizeStr[4] = {
	"8", "16", "32", "??"
};
static const char *onOffStr[2] = {
	"OFF", "ON "
};
static const char *numbersStr[8] = {
	"0", "1", "2", "3", "4", "5", "6", "7"
};
static const char *cpuSpeedStr[8] = {
	"3.58MHz", "4.10MHz", "4.48MHz", "4.90MHz", "5.39MHz", "6.10MHz", "6.96MHz", "8.06MHz"
};
static const char *extBusStr[2] = {
	"CPU Spd", "3.58MHz"
};
static const char *keyboardStr[2] = {
	"JP    ", "Non-JP"
};
static const char *videoTypeStr[2] = {
	"Forced", "Auto  "
};
static const char *forcedVideoStr[2] = {
	"60Hz(NTSC)", "50Hz(PAL) "
};
static const char *legacyVgaStr[2] = {
	"VGA ", "VGA+"
};
static const char *scanlinesStr[4] = {
	"0% ", "25%", "50%", "75%"
};
static const char *dipCpuStr[2] = {
	"3.58MHz", "Custom "
};
static const char *dipVideoStr[4] = {
	"Cmp/S-Vid", "VGA 1:1  ", "RGB 15KHz", "VGA+     "
};
static const char *dipSlot1Str[2] = {
	"External", "Internal"
};
static const char *dipSlot2Str[4] = {
	"External", "Int/SCCI", "Int/A8K ", "Int/A16K"
};
static const char *dipMapperStr[2] = {
	"2048Kb", "4096Kb"
};

static const Element_t elemSystem[] = {
	// 0
	{
		LABEL, 
		3,5,  "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Frequencies \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 1
	{	
		SLIDER,
		3,7, " CPU Speed ",
		4, 1, 6, 6,
		&(sysInfo0.raw), 0b00000111, cpuSpeedStr, 12, 
		{ OCM_SMART_CPU358MHz, OCM_SMART_CPU410MHz, OCM_SMART_CPU448MHz, OCM_SMART_CPU490MHz, 
			OCM_SMART_CPU539MHz, OCM_SMART_CPU610MHz, OCM_SMART_CPU696MHz, OCM_SMART_CPU806MHz }, true,
		{ "Custom CPU speed. These are not real CPU frequencies but a simulation",
			"slowing down the 10.74MHz frequency. The values have been calculated and",
			"are purely indicative. The default is 8.06MHz.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,9, " Ext.Bus Clock ",
		-1, 1, 6, 6,
		&(sysInfo2.raw), 0b00000010, extBusStr, 18,
		{ OCM_SMART_ExtBusCPU, OCM_SMART_ExtBus358 }, true,
		{ "Synchronize external bus clock to CPU speed (default), or force external",
			"bus clock to 3.58MHz.", (char*)NULL },
		IOREV_3, M_ALL
	},
	// 3
	{
		SLIDER,
		3,11, " Turbo Pana ",
		-1, 1, 5, 5,
		&(sysInfo1.raw), 0b00000001, onOffStr, 18,
		{ OCM_SMART_RestoreTurbo, OCM_SMART_TurboPana }, true,
		{ "Turbo Pana(sonic) sets CPU clock to 5.37MHz. It differs from Custom speed",
			"5.39MHz in that CPU & External clock run at different speeds between the",
			"two modes, and Turbo Pana does not involve simulation by slowdown.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 4
	{
		SLIDER,
		3,13, " T. Pana redir. ",
		-1, 1, 4, 4,
		&(sysInfo0.raw), 0b00010000, onOffStr, 18,
		{ OCM_SMART_TPanaRedOFF, OCM_SMART_TPanaRedON }, true,
		{ "Turbo Pana redirect mode: sets Turbo Pana I/O to the current Custom Speed.",
			"This allows games to reach 8.06MHz or other frequencies using port 41h ID8",
			"making them OCM-compatible without software patch.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 5
	{
		SLIDER,
		3,15, " Turbo MegaSD ",
		-1, -4, 3, 3,
		&(sysInfo0.raw), 0b00001000, onOffStr, 18,
		{ OCM_SMART_TMegaSDOFF, OCM_SMART_TMegaSDON }, true,
		{ "Turbo MegaSD sets SDCard speed access at loading time activating 8.06MHz,",
			"so you get fast load even with CPU to 3.58/5.37MHz. It can adversely affect",
			"external cartridges that do not support 8.06MHz.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 6
	{
		LABEL, 
		40,5, "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Keyboard \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 7
	{
		VALUE,
		40,7, " Default Keyboard ",
		1, 1, -6, -6,
		&(pldVers1.raw), 0b10000000, keyboardStr, 28,
		{ 0x00 }, false,
		{ "Default keyboard layout (read only).",
			"Values can be Japanese or non-Japanese. The latter is the international",
			"keyboard layout embedded inside the BIOS.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 8
	{
		SLIDER,
		40,9, " Active Keyboard ",
		-1, -1, -6, -6,
		&(sysInfo1.raw), 0b00000010, keyboardStr, 20,
		{ OCM_SMART_KBLayoutJP, OCM_SMART_KBLayoutNJP }, false,
		{ "Current active keyboard layout.",
			"Values can be Japanese or non-Japanese. The latter is the international",
			"keyboard layout embedded inside the BIOS.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// END
	{ END }
};

static const Element_t elemVideo[] = {
	// 0
	{
		SLIDER,
		3,6, " VDP Fast ",
		5, 1, 0, 0,
		&(sysInfo0.raw), 0b00100000, onOffStr, 24,
		{ OCM_SMART_VDPNormal, OCM_SMART_VDPFast }, false,
		{ "OFF: Works like real hardware (default).",
			"ON:  The VDP works faster (V9958 only).", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 1
	{
		SLIDER,
		3,8, " Video mode ",
		-1, 1, 0, 0,
		&(sysInfo2.raw), 0b01000000, videoTypeStr, 24,
		{ OCM_SMART_ForceNTSC, OCM_SMART_VideoAuto }, false,
		{ "Force video output mode (NTSC/PAL), or set to auto (default) that is bound",
			"by VDP Control Register #9.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 2
	{
		SLIDER,
		3,10, " Forced video mode ",
		-1, 1, 0, 0,
		&(sysInfo2.raw), 0b10000000, forcedVideoStr, 24,
		{ OCM_SMART_ForceNTSC, OCM_SMART_ForcePAL }, false,
		{ "Set forced video output mode to NTSC/PAL.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 3
	{
		SLIDER,
		3,12, " Legacy Output ",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00100000, legacyVgaStr, 24,
		{ OCM_SMART_LegacyVGA, OCM_SMART_LegacyVGAplus }, false,
		{ "Assign Legacy Output to VGA or VGA+ (default).", (char*)NULL },
		IOREV_8, M_ALL
	},
	// 4
	{
		SLIDER,
		3,14, " Center YJK modes",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00010000, onOffStr, 24,
		{ OCM_SMART_CenterYJKOFF, OCM_SMART_CenterYJKON }, false,
		{ "Allows to force the centering of YJK modes and VDP R#25 mask, useful for",
			"MSX2+ games. Default is OFF.", (char*)NULL },
		IOREV_7, M_ALL
	},
	// 5
	{
		SLIDER,
		3,16, " Scanlines VGA/VGA+",
		-1, -5, 0, 0,
		&(sysInfo4.raw), 0b00000011, scanlinesStr, 22,
		{ OCM_SMART_Scanlines00, OCM_SMART_Scanlines25, OCM_SMART_Scanlines50, 
			OCM_SMART_Scanlines75 }, false,
		{ "Visualization of scanlines for VGA/VGA+ (Mainly for 2nd Gen machines).", (char*)NULL },
		IOREV_ALL, M_SX2|M_SMX_MCP2_ID
	},
	// END
	{ END }
};

static const Element_t elemAudio[] = {
	// 0
	{
		SLIDER,
		3,6, " Master volume ",
		5, 1, 6, 6,
		&(audioVols0.raw), 0b01110000, numbersStr, 18,
		{ OCM_SMART_MasterVol0, OCM_SMART_MasterVol1, OCM_SMART_MasterVol2,
			OCM_SMART_MasterVol3, OCM_SMART_MasterVol4, OCM_SMART_MasterVol5,
			OCM_SMART_MasterVol6, OCM_SMART_MasterVol7 }, false,
		{ "Set Master volume level.",
			"Default is 7", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 1
	{
		SLIDER,
		3,8, " PSG volume ",
		-1, 1, 6, 6,
		&(audioVols0.raw), 0b00000111, numbersStr, 18,
		{ OCM_SMART_PSGVol0, OCM_SMART_PSGVol1, OCM_SMART_PSGVol2,
			OCM_SMART_PSGVol3, OCM_SMART_PSGVol4, OCM_SMART_PSGVol5,
			OCM_SMART_PSGVol6, OCM_SMART_PSGVol7 }, false,
		{ "Set PSG volume level.",
			"Default is 4", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,10, " SCC-I volume ",
		-1, 1, 5, 5,
		&(audioVols1.raw), 0b01110000, numbersStr, 18,
		{ OCM_SMART_SCCIVol0, OCM_SMART_SCCIVol1, OCM_SMART_SCCIVol2,
			OCM_SMART_SCCIVol3, OCM_SMART_SCCIVol4, OCM_SMART_SCCIVol5,
			OCM_SMART_SCCIVol6, OCM_SMART_SCCIVol7 }, false,
		{ "Set SCC-I volume level.",
			"Default is 4", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 3
	{
		SLIDER,
		3,12, " OPLL volume ",
		-1, 1, 4, 4,
		&(audioVols1.raw), 0b00000111, numbersStr, 18,
		{ OCM_SMART_OPLLVol0, OCM_SMART_OPLLVol1, OCM_SMART_OPLLVol2,
			OCM_SMART_OPLLVol3, OCM_SMART_OPLLVol4, OCM_SMART_OPLLVol5,
			OCM_SMART_OPLLVol6, OCM_SMART_OPLLVol7 }, false,
		{ "Set OPLL volume level.",
			"Default is 4", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,14, " PSG2 ",
		-1, 1, 3, 3,
		&(sysInfo4.raw), 0b00000100, onOffStr, 24,
		{ OCM_SMART_IntPSG2OFF, OCM_SMART_IntPSG2ON }, false,
		{ "Enable/disable an additional internal PSG (acting as an external PSG).",
			"Default is OFF", (char*)NULL },
		IOREV_11, M_SX2|M_SMX_MCP2_ID
	},
	// 5
	{
		SLIDER,
		3,16, " OPL3 ",
		-1, -5, 2, 2,
		&(sysInfo1.raw), 0b00000100, onOffStr, 24,
		{ OCM_SMART_OPL3OFF, OCM_SMART_OPL3ON }, false,
		{ "Enable/disable the OPL3 sound chipset.",
			"Default is OFF", (char*)NULL },
		IOREV_10, M_SX2|M_SMX_MCP2_ID
	},
	// 6
	{
		SLIDER,
		40,6, " Pseudo stereo ",
		1, 1, -6, -6,
		&(sysInfo2.raw), 0b00000001, onOffStr, 24,
		{ OCM_SMART_PseudSterOFF, OCM_SMART_PseudSterON }, false,
		{ "Enable/disable the Pseudo-Stereo mode (needs an external sound cartridge).",
			"Default is OFF", (char*)NULL },
		IOREV_3, M_ALL
	},
	// 7
	{
		SLIDER,
		40,8, " Right Inverse Audio ",
		-1, -1, -6, -6,
		&(sysInfo3.raw), 0b00000001, onOffStr, 24,
		{ OCM_SMART_RightInvAud0, OCM_SMART_RightInvAud1 }, false,
		{ "Enable/disable the Right Inverse Audio, is a good solution for recording",
			"really clean balanced audio.",
			"Default is OFF", (char*)NULL },
		IOREV_5, M_ALL
	},
	// END
	{ END }
};

static const Element_t elemDIPs[] = {
	// 0
	{
		LABEL, 
		3,5,  "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Virtual DIPs \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 1
	{	
		SLIDER,
		3,7, " CPU Clock ",
		5, 1, 7, 7,
		&(virtualDIPs.raw), 0b00000001, dipCpuStr, 22, 
		{ 0x00 }, false,
		{ "Virtual DIP-Switch #1: CPU Clock",
			"OFF: CPU 3.58MHz",
			"ON:  Custom Speed mode 4.10MHz to 8.06MHz", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,9, " Video Output ",
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b00000110, dipVideoStr, 20, 
		{ OCM_SMART_Disp15KhSvid, OCM_SMART_Disp31KhVGA, OCM_SMART_Disp15KhRGB, OCM_SMART_Disp31KhVGAp }, false,
		{ "Virtual DIP-Switch #2-#3: Video Output",
			"OFF/OFF: Composite/S-Video w/mono audio     OFF/ON: RGB 15khz",
			"ON/OFF:  VGA Mode w/Pixel 1:1               ON/ON:  VGA+ Mode for CRT", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 3
	{
/*REV*/	VALUE,
		3,11, " Cartridge Slot 1 ",
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b00001000, dipSlot1Str, 30, 
/*REV*/	{ 0x00 }, false,
		{ "Virtual DIP-Switch #4: Cartridge Slot 1 Configuration",
			"OFF: External Slot-1 / Optional Slot-3 (shared)",
			"ON:  Internal ESE-MegaSCC+ 1Mb (shared w/2nd half ESE-MegaSCC+ Slot-2)", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
/*REV*/	VALUE,
		3,13, " Cartridge Slot 2 ",
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b00110000, dipSlot2Str, 30, 
/*REV*/	{ 0x00 }, false,
		{ "Virtual DIP-Switch #5-#6: Cartridge Slot 2 Configuration",
			"OFF/OFF: External Slot-2           OFF/ON: Internal ESE-MegaRAM ASCII-8 1Mb",
			"ON/OFF:  Internal ESE-MegaSCC+ 2Mb ON/ON:  Internal ESE-MegaRAM ASCII-16 2Mb", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 5
	{
		SLIDER,
		3,15, " RAM Mapper ",
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b01000000, dipMapperStr, 22, 
/*REV*/	{ OCM_SMART_Mapper4MbOFF, OCM_SMART_Mapper4MbON }, false,	// Need warm reset? 0xfc 0xfe
		{ "Virtual DIP-Switch #7: RAM Mapper",
			"OFF: Internal 2048Kb RAM / 1st EPBIOS",
			"ON:  Internal 4096Kb RAM / Optional 2nd EPBIOS", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 6
	{
		SLIDER,
		3,17, " Internal MegaSD ",
		-1, -5, 7, 7,
		&(virtualDIPs.raw), 0b10000000, onOffStr, 22, 
/*REV*/	{ OCM_SMART_MegaSDOFF, OCM_SMART_MegaSDON }, false,			// Need warm reset? 0xfd
		{ "Virtual DIP-Switch #8: SD Card Slot",
			"OFF: Disabled",
			"ON:  Enabled", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 7
	{
		LABEL, 
		44,5, "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Hard DIPs \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 8
	{	
		VALUE,
		44,7, " CPU Clock ",
		5, 1, -7, -7,
		&(sysInfo5.raw), 0b00000001, dipCpuStr, 24, 
		{ 0x00 }, false,
		{ "Hardware DIP-Switch #1: CPU Clock",
			"OFF: CPU 3.58MHz",
			"ON:  Custom Speed mode 4.10MHz to 8.06MHz", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 9
	{
		VALUE,
		44,9, " Video Output ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00000110, dipVideoStr, 24, 
		{ 0x00 }, false,
		{ "Hardware DIP-Switch #2-#3: Video Output",
			"OFF/OFF: Composite/S-Video w/mono audio     OFF/ON: RGB 15khz",
			"ON/OFF:  VGA Mode Pixel 1:1                 ON/ON:  VGA+ Mode for CRT", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 10
	{
		VALUE,
		44,11, " Cartridge Slot 1 ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00001000, dipSlot1Str, 24, 
		{ 0x00 }, false,
		{ "Hardware DIP-Switch #4: Cartridge Slot 1 Configuration",
			"OFF: External Slot-1 / Optional Slot-3 (shared)",
			"ON:  Internal ESE-MegaSCC+ 1Mb (shared w/2nd half ESE-MegaSCC+ Slot-2)", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 11
	{
		VALUE,
		44,13, " Cartridge Slot 2 ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00110000, dipSlot2Str, 24, 
		{ 0x00 }, false,
		{ "Hardware DIP-Switch #5-#6: Cartridge Slot 2 Configuration",
			"OFF/OFF: External Slot-2           OFF/ON: Internal ESE-MegaRAM ASCII-8 1Mb",
			"ON/OFF:  Internal ESE-MegaSCC+ 2Mb ON/ON:  Internal ESE-MegaRAM ASCII-16 2Mb", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 12
	{
		VALUE,
		44,15, " RAM Mapper ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b01000000, dipMapperStr, 24, 
		{ 0x00 }, false,
		{ "Hardware DIP-Switch #7: RAM Mapper",
			"OFF: Internal 2048Kb RAM / 1st EPBIOS",
			"ON:  Internal 4096Kb RAM / Optional 2nd EPBIOS", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 13
	{
		VALUE,
		44,17, " Internal MegaSD ",
		-1, -5, -7, -7,
		&(sysInfo5.raw), 0b10000000, onOffStr, 24, 
		{ 0x00 }, false,
		{ "Hardware DIP-Switch #8: SD Card Slot",
			"OFF: Disabled",
			"ON:  Enabled", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// END
	{ END }
};

static const Element_t elemHelp[] = {
	{ LABEL, 3,5,  "     --==[ OCMINFO.COM & CONIO.LIB ]==-- by NataliaPC (@ishwin74) '2024     ",
		0, 0, 0, 0, NULL, 0, NULL, 0, { 0x00 }, false, 
		{ "Thanks to: @KdL, @Ducasp, and @Cayce-msx",
			"",
			"GitHub: https://github.com/nataliapc", (char*)NULL }
	},
	{ LABEL, 3,7,  "This program aims to be a helpful tool for configuring and maintaining" },
	{ LABEL, 3,8,  "MSX++ (OCM-PLD) FPGA-based systems." },
	{ LABEL, 3,10, "Use the function keys to navigate between different panels and the cursor" },
	{ LABEL, 3,11, "keys to select the items. Information text will appear at bottom." },
	{ LABEL, 3,12, "To modify an item, you can press SPACE, ENTER, or '+' to increase its value" },
	{ LABEL, 3,13, "or '-' to decrease it. TAB key to navigate to next panel." },
	{ LABEL, 3,15, "If you want to suggest improvements, feel free to create an issue at the" },
	{ LABEL, 3,16, "GitHub project page, the link is shown below." },
	// END
	{ END }
};

enum {
	PANEL_SYSTEM,
	PANEL_VIDEO,
	PANEL_AUDIO,
	PANEL_DIPS,
	PANEL_HELP
};
static const Panel_t pPanels[] = {
	{ " F1:System ",	3,3, 	11, elemSystem },
	{ " F2:Video ",		15,3, 	10, elemVideo },
	{ " F3:Audio ",		26,3,	10, elemAudio },
	{ " F5:DIP-SW ",	48,3,	11, elemDIPs },
	{ " H:Help ",		63,3,	8, elemHelp },
	{ " Q:Exit ",		71,3,	8, NULL },
	{ NULL }
};


// ========================================================
static void drawCurrentPanel();
void _fillVRAM(uint16_t vram, uint16_t len, uint8_t value) __sdcccall(0);


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
		currentElement->cmd[0] == 0x00) {
		putch('\x07');
		return false;
	}

	uint8_t maxValue = getMaxValue(currentElement);
	int8_t value = getValue(currentElement);
	value += increase;
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
		putlinexy(34,19, strlen(heap_top), heap_top);
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
static void drawSlider(Element_t *element, uint8_t posx, uint8_t value)
{
	char sliderStr[] = "========";
	uint16_t len = getMaxValue(element) + 1;
	sliderStr[len] = '\0';
	if (element->valueStr != NULL) {
		csprintf(heap_top, "-[%s]+  %s", sliderStr, element->valueStr[value]);
	} else {
		csprintf(heap_top, "-[%s]+  %u  ", sliderStr, value);
	}
	putlinexy(posx, wherey(), strlen(heap_top), heap_top);
	putlinexy(posx+2+value, wherey(), 1, "\x85");
}

static bool drawElement(Element_t *element)
{
	if (element->type == END) return false;

	uint8_t value = getValue(element);
	uint8_t posx = element->x;
	uint8_t posy = element->y;

	putlinexy(posx, posy, strlen(element->label), element->label);

	if (element->type == LABEL) return true;

	if (!isIOrevisionSupported(element) || !isMachineSupported(element)) {
		putlinexy(posx+element->valueOffsetX+4, posy, 3, "n/a");
		return true;
	}

	posx += element->valueOffsetX;
	gotoxy(posx, posy);

	switch (element->type) {
		case LABEL:
			break;
		case VALUE:
			if (element->valueStr == NULL) {
				csprintf(heap_top, "%u", value);
			} else {
				csprintf(heap_top, "%s", element->valueStr[value]);
			}
			putlinexy(wherex(), wherey(), strlen(heap_top), heap_top);
			break;
		case ONOFF:
			putlinexy(wherex(), wherey(), strlen(element->valueStr[value]), element->valueStr[value]);
			break;
		case SLIDER:
			drawSlider(element, posx, value);
			break;
	}

	return true;
}

static void selectCurrentElement(bool enable)
{
	textblink(
		currentElement->x, currentElement->y, 
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
		textblink(currentElement->x, currentElement->y, strlen(currentElement->label), false);
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

	// A way to avoid using low memory parameters for BIOS calls.
	if (heap_top < (void*)0x8000)
		heap_top = (void*)0x8000;

	//Platform system checks
//	checkPlatformSystem();

	// Initialize screen 0[80]
	textmode(BW80);
	textattr(0xa1f4);
	setcursortype(NOCURSOR);
	redefineFunctionKeys();

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
				putlinexy(34,19, 12, "            ");
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
//				case '4':
//					selectPanel(&pSlots);
//					break;
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
	clrscr();
	_fillVRAM(0x1b00, 240, 0);
	__asm
		push ix
		ld ix, #INIFNK
		BIOSCALL
		pop ix
	__endasm;
	exit();
}
