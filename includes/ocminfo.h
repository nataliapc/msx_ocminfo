#include <stdint.h>
#include <stdbool.h>


// ========================================================
// Defines

#define VERSION		"0.9.4"

#define SETSMART_X	34
#define SETSMART_Y	19


// ========================================================
// External hidden functions

void setByteVRAM(uint16_t vram, uint8_t value) __sdcccall(0);
void _fillVRAM(uint16_t vram, uint16_t len, uint8_t value) __sdcccall(0);


// ========================================================
// Structs & Enums

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
	LABEL,									// No widget, just label
	VALUE,									// Show numeric value
	SLIDER,									// Slider with ranges of 0 to 2/4/8 values
	END,									// **End of Elements array
	// Custom widgets
	CUSTOM_CPUSPEED,
} Widget_t;

typedef enum {
	CMDTYPE_NONE,
	CMDTYPE_STANDARD,
	CMDTYPE_CUSTOM_SLOTS12,
} CmdType_t;

enum {										// Masks for Element_t.attribs.raw
	ATR_FORCEPANELRELOAD = 1,
	ATR_NEEDRESETTOAPPLY = 2,
	ATR_UNUSED6          = 8,
	ATR_UNUSED5          = 8,
	ATR_UNUSED4          = 16,
	ATR_UNUSED3          = 32,
	ATR_UNUSED2          = 64,
	ATR_UNUSED1          = 128,
};

typedef struct {
	Widget_t type;							// Widget type
	uint8_t posX, posY;						// Element position
	char *label;							// Element text label
	int8_t goUp, goDown, goLeft, goRight;	// Navigation to indexes (relative offset)
	uint8_t *value;							// Cached full byte for value
	uint8_t valueMask;						// Mask for value
	uint8_t minValue;						// Min value
	uint8_t maxValue;						// Max value
	uint8_t **valueStr;						// Array with labels for each value
	uint8_t valueOffsetX;					// Offset X for value widget
	CmdType_t cmdType;						// OCM Smart Cmd type mode
	uint8_t cmd[8];							// OCM Smart Cmd to set each value
	union {
		uint8_t attribs_raw;
		struct {
			unsigned forcePanelReload: 1;	// Force panel reload when value changes
			unsigned needResetToApply: 1;	// Need a Cold/Warm reset to apply
			unsigned reserved: 6;			// Not used flags [reserved]
		};
	};
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
// Text constants

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
static const char *cpuSpeedStr[9] = {
	"4.10MHz", "4.48MHz", "4.90MHz", "5.39MHz", "6.10MHz", "6.96MHz", "8.06MHz",// Custom clocks
	"3.58MHz", "5.37MHz"														// Standard clocks
};
static const char *cpuModeStr[9] = {
	"Standard", "tPANA   ", "Custom  "
};
static const char *customSpeedStr[8] = {
	"", "4.10MHz", "4.48MHz", "4.90MHz", "5.39MHz", "6.10MHz", "6.96MHz", "8.06MHz"
};
static const char *extBusStr[2] = {
	"CPU Clk", "3.58MHz"
};
static const char *keyboardStr[2] = {
	"JP    ", "Non-JP"
};
static const char *videoModeStr[3] = {
	"Auto      ", "NTSC(60Hz)", "PAL(50Hz) "
};
static const char *legacyVgaStr[2] = {
	"VGA ", "VGA+"
};
static const char *scanlinesStr[4] = {
	"0% ", "25%", "50%", "75%"
};
static const char *dipCpuStr[2] = {
	"Standard", "Custom  "
};
static const char *dipVideoStr[4] = {
	"Cmp/S-Vid", "VGA 1:1  ", "RGB 15KHz", "VGA+     "
};
static const char *dipSlot1Str[2] = {
	"External", "Int.SCC+"
};
static const char *dipSlot2Str[4] = {
	"External", "Int.SCC+", "Int.A8K ", "Int.A16K"
};
static const char *dipMapperStr[2] = {
	"2048Kb", "4096Kb"
};


// ========================================================
// PanelElements constants

static const Element_t elemSystem[] = {
	// 0
	{
		LABEL, 
		3,5,  "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Frequencies \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 1
	{
		CUSTOM_CPUSPEED,
		3,7, " CPU Speed ",
		5, 1, 7, 7,
		&customCpuSpeedValue, 0b00001111, 0,10, cpuSpeedStr, 28,
		CMDTYPE_NONE,
		{ 0x00 }, 
		false,
		{ "Currently active CPU clock frequency (read only).", 
		  "Customize below values to adjust it.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,8, " CPU Mode ",
		-1, 1, 7, 7,
		&customCpuModeValue, 0b00000011, 0,2, cpuModeStr, 19,
		CMDTYPE_STANDARD, 
		{ OCM_SMART_CPU358MHz, OCM_SMART_TurboPana, OCM_SMART_CPU806MHz }, 
		ATR_FORCEPANELRELOAD,
		{ "Standard:   Standard CPU Speed mode (3.58MHz)",
		  "Turbo Pana: Panasonic turbo mode (aka tPANA, 5.37MHz)",
		  "Custom:     Custom CPU Speed mode (4.10MHz to 8.06MHz)", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,9, " Custom Speed ",
		-1, 1, 6, 6,
		&(sysInfo0.raw), 0b00000111, 1,7, customSpeedStr, 14,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CPU410MHz, OCM_SMART_CPU410MHz, OCM_SMART_CPU448MHz, OCM_SMART_CPU490MHz, 
		  OCM_SMART_CPU539MHz, OCM_SMART_CPU610MHz, OCM_SMART_CPU696MHz, OCM_SMART_CPU806MHz }, 
		ATR_FORCEPANELRELOAD,
		{ "Custom CPU Speed. These are not real CPU frequencies but a simulation",
		  "slowing down the 10.74MHz frequency. The values have been calculated and",
		  "are purely indicative. The default is 8.06MHz.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 5
	{
		SLIDER,
		3,10, " Ext.Bus Clock ",
		-1, 1, 5, 5,
		&(sysInfo2.raw), 0b00000010, 0,1, extBusStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ExtBusCPU, OCM_SMART_ExtBus358 },
		ATR_FORCEPANELRELOAD,
		{ "Synchronize external bus clock to CPU Speed (default), or force it to",
		  "3.58MHz.", (char*)NULL },
		IOREV_3, M_ALL
	},
	// 6
	{
		SLIDER,
		3,12, " T. Pana redir. ",
		-1, 1, 4, 4,
		&(sysInfo0.raw), 0b00010000, 0,1, onOffStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_TPanaRedOFF, OCM_SMART_TPanaRedON },
		false,
		{ "Turbo Pana redirect mode: sets Turbo Pana I/O to the current Custom Speed.",
		  "This allows games to reach 8.06MHz or other frequencies using port 41h ID8",
		  "making them OCM-compatible without software patch.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 7
	{
		SLIDER,
		3,13, " Turbo MegaSD ",
		-1, -5, 3, 3,
		&(sysInfo0.raw), 0b00001000, 0,1, onOffStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_TMegaSDOFF, OCM_SMART_TMegaSDON },
		false,
		{ "Turbo MegaSD sets SDCard speed access at loading time activating 8.06MHz,",
		  "so you get fast load even with CPU to 3.58/5.37MHz. It can adversely affect",
		  "external cartridges that do not support 8.06MHz.", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 8
	{
		LABEL, 
		42,5, "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Keyboard \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 9
	{
		VALUE,
		42,7, " Default Keyboard ",
		1, 1, -7, -7,
		&(pldVers1.raw), 0b10000000, 0,1, keyboardStr, 28,
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ "Default keyboard layout (read only).",
		  "Values can be Japanese or non-Japanese (international keyboard layout",
		  "embedded inside the BIOS).", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 10
	{
		SLIDER,
		42,8, " Current Keyboard ",
		-1, -1, -7, -7,
		&(sysInfo1.raw), 0b00000010, 0,1, keyboardStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_KBLayoutJP, OCM_SMART_KBLayoutNJP },
		false,
		{ "Current active keyboard layout.",
		  "Values can be Japanese or non-Japanese (international keyboard layout",
		  "embedded inside the BIOS).", (char*)NULL },
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
		4, 1, 0, 0,
		&(sysInfo0.raw), 0b00100000, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VDPNormal, OCM_SMART_VDPFast },
		false,
		{ "OFF: Works like real hardware (default).",
		  "ON:  The VDP works faster (V9958 only).", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 1
	{
		SLIDER,
		3,8, " Video Mode ",
		-1, 1, 0, 0,
		&customVideoModeValue, 0b00000011, 0,2, videoModeStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VideoAuto, OCM_SMART_ForceNTSC, OCM_SMART_ForcePAL },
		ATR_FORCEPANELRELOAD,
		{ "Video output mode (Auto/PAL/NTSC)", 
		  "Auto:     set to auto (default) that is bound by VDP Control Register #9.",
		  "NTSC/PAL: force video output to NTSC(60Hz) or PAL(50Hz).", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 2
	{
		SLIDER,
		3,10, " Legacy Output ",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00100000, 0,1, legacyVgaStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_LegacyVGA, OCM_SMART_LegacyVGAplus },
		false,
		{ "Assign Legacy Output to VGA or VGA+ (default).", (char*)NULL },
		IOREV_8, M_ALL
	},
	// 3
	{
		SLIDER,
		3,12, " Center YJK modes",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00010000, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CenterYJKOFF, OCM_SMART_CenterYJKON },
		false,
		{ "Allows to force the centering of YJK modes and VDP R#25 mask, useful for",
		  "MSX2+ games. Default is OFF.", (char*)NULL },
		IOREV_7, M_ALL
	},
	// 4
	{
		SLIDER,
		3,14, " Scanlines VGA/VGA+",
		-1, -4, 0, 0,
		&(sysInfo4.raw), 0b00000011, 0,3, scanlinesStr, 22,
		CMDTYPE_STANDARD,
		{ OCM_SMART_Scanlines00, OCM_SMART_Scanlines25, OCM_SMART_Scanlines50, OCM_SMART_Scanlines75 },
		false,
		{ "Visualization of scanlines for VGA/VGA+ (mainly for 2nd Gen machines).", (char*)NULL },
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
		&(audioVols0.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_MasterVol0, OCM_SMART_MasterVol1, OCM_SMART_MasterVol2, OCM_SMART_MasterVol3,
		  OCM_SMART_MasterVol4, OCM_SMART_MasterVol5, OCM_SMART_MasterVol6, OCM_SMART_MasterVol7 },
		false,
		{ "Set Master volume level.",
		  "Default is 7", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 1
	{
		SLIDER,
		3,7, " PSG volume ",
		-1, 1, 5, 5,
		&(audioVols0.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PSGVol0, OCM_SMART_PSGVol1, OCM_SMART_PSGVol2, OCM_SMART_PSGVol3,
		  OCM_SMART_PSGVol4, OCM_SMART_PSGVol5, OCM_SMART_PSGVol6, OCM_SMART_PSGVol7 },
		false,
		{ "Set PSG volume level.",
		  "Default is 4", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,8, " SCC+ volume ",
		-1, 1, 5, 5,
		&(audioVols1.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_SCCIVol0, OCM_SMART_SCCIVol1, OCM_SMART_SCCIVol2, OCM_SMART_SCCIVol3,
		  OCM_SMART_SCCIVol4, OCM_SMART_SCCIVol5, OCM_SMART_SCCIVol6, OCM_SMART_SCCIVol7 },
		false,
		{ "Set SCC+ volume level.",
		  "Default is 4", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 3
	{
		SLIDER,
		3,9, " OPLL volume ",
		-1, 1, 4, 4,
		&(audioVols1.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPLLVol0, OCM_SMART_OPLLVol1, OCM_SMART_OPLLVol2, OCM_SMART_OPLLVol3, 
		  OCM_SMART_OPLLVol4, OCM_SMART_OPLLVol5, OCM_SMART_OPLLVol6, OCM_SMART_OPLLVol7 },
		false,
		{ "Set OPLL volume level.",
		  "Default is 4", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,11, " PSG2 ",
		-1, 1, 3, 3,
		&(sysInfo4.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_IntPSG2OFF, OCM_SMART_IntPSG2ON },
		false,
		{ "Enable/disable an additional internal PSG (acting as an external PSG).",
		  "Default is OFF", (char*)NULL },
		IOREV_11, M_SX2|M_SMX_MCP2_ID
	},
	// 5
	{
		SLIDER,
		3,12, " OPL3 ",
		-1, -5, 2, 2,
		&(sysInfo1.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPL3OFF, OCM_SMART_OPL3ON },
		false,
		{ "Enable/disable the OPL3 sound chipset.",
		  "Default is OFF", (char*)NULL },
		IOREV_10, M_SX2|M_SMX_MCP2_ID
	},
	// 6
	{
		SLIDER,
		40,6, " Pseudo stereo ",
		1, 1, -6, -6,
		&(sysInfo2.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PseudSterOFF, OCM_SMART_PseudSterON },
		false,
		{ "Enable/disable the Pseudo-Stereo mode (needs an external sound cartridge).",
		  "Default is OFF", (char*)NULL },
		IOREV_3, M_ALL
	},
	// 7
	{
		SLIDER,
		40,8, " Right Inverse Audio ",
		-1, -1, -5, -5,
		&(sysInfo3.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_RightInvAud0, OCM_SMART_RightInvAud1 },
		false,
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
		&(virtualDIPs.raw), 0b00000001, 0,1, dipCpuStr, 22, 
		CMDTYPE_STANDARD,
		{ OCM_SMART_CPU358MHz, OCM_SMART_CPU806MHz },
		ATR_FORCEPANELRELOAD,
		{ "Virtual DIP-Switch #1: CPU Clock",
		  "OFF: Standard CPU clock mode (3.58MHz)",
		  "ON:  Custom CPU clock mode (4.10MHz to 8.06MHz)", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,9, " Video Output ",
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b00000110, 0,3, dipVideoStr, 20, 
		CMDTYPE_STANDARD,
		{ OCM_SMART_Disp15KhSvid, OCM_SMART_Disp31KhVGA, OCM_SMART_Disp15KhRGB, OCM_SMART_Disp31KhVGAp },
		ATR_FORCEPANELRELOAD,
		{ "Virtual DIP-Switch #2-#3: Video Output",
		  "OFF/OFF: Composite/S-Video w/mono audio     OFF/ON: RGB 15khz",
		  "ON/OFF:  VGA Mode w/Pixel 1:1               ON/ON:  VGA+ Mode for CRT", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 3
	{
/*REV*/	SLIDER,
		3,11, " Cartridge Slot 1 ",
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000001, 0,1, dipSlot1Str, 22, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2a16, OCM_SMART_S1sccS2a16 },
		ATR_FORCEPANELRELOAD,
		{ "Virtual DIP-Switch #4: Cartridge Slot 1 Configuration",
		  "OFF: External Slot-1 / Optional Slot-3 (shared)",
		  "ON:  Internal ESE-MegaSCC+", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
/*REV*/	SLIDER,
		3,13, " Cartridge Slot 2 ",
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000110, 0,3, dipSlot2Str, 20, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2a16, OCM_SMART_S1sccS2a16 },
		ATR_FORCEPANELRELOAD,
		{ "Virtual DIP-Switch #5-#6: Cartridge Slot 2 Configuration",
		  "OFF/OFF: External Slot-2          OFF/ON: Internal ESE-MegaRAM ASCII-8K",
		  "ON/OFF:  Internal ESE-MegaSCC+    ON/ON:  Internal ESE-MegaRAM ASCII-16K", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 5
	{
		VALUE,
		3,15, " RAM Mapper ",
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b01000000, 0,1, dipMapperStr, 30,//22,
		CMDTYPE_NONE,
/*REV*/	/*{ OCM_SMART_Mapper4MbOFF, OCM_SMART_Mapper4MbON },
		ATR_FORCEPANELRELOAD | ATR_NEEDRESETTOAPPLY,*/ {0x00}, false,
		{ "Virtual DIP-Switch #7: RAM Mapper",
		  "OFF: Internal 2048Kb RAM / 1st EPBIOS",
		  "ON:  Internal 4096Kb RAM / Optional 2nd EPBIOS", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 6
	{
		VALUE,
		3,17, " Internal MegaSD ",
		-1, -5, 7, 7,
		&(virtualDIPs.raw), 0b10000000, 0,1, onOffStr, 30,//22,
		CMDTYPE_NONE,
/*REV*/	/*{ OCM_SMART_MegaSDOFF, OCM_SMART_MegaSDON },
		ATR_FORCEPANELRELOAD | ATR_NEEDRESETTOAPPLY,*/ {0x00}, false,
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
		&(sysInfo5.raw), 0b00000001, 0,1, dipCpuStr, 24, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ "Hardware DIP-Switch #1: CPU Clock",
		  "OFF: Standard CPU clock mode (3.58MHz)",
		  "ON:  Custom CPU clock mode (4.10MHz to 8.06MHz)", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 9
	{
		VALUE,
		44,9, " Video Output ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00000110, 0,3, dipVideoStr, 24, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ "Hardware DIP-Switch #2-#3: Video Output",
		  "OFF/OFF: Composite/S-Video w/mono audio     OFF/ON: RGB 15khz",
		  "ON/OFF:  VGA Mode w/Pixel 1:1               ON/ON:  VGA+ Mode for CRT", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 10
	{
		VALUE,
		44,11, " Cartridge Slot 1 ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00001000, 0,1, dipSlot1Str, 24, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ "Hardware DIP-Switch #4: Cartridge Slot 1 Configuration",
		  "OFF: External Slot-1 / Optional Slot-3 (shared)",
		  "ON:  Internal ESE-MegaSCC+", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 11
	{
		VALUE,
		44,13, " Cartridge Slot 2 ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00110000, 0,3, dipSlot2Str, 24, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ "Hardware DIP-Switch #5-#6: Cartridge Slot 2 Configuration",
		  "OFF/OFF: External Slot-2          OFF/ON: Internal ESE-MegaRAM ASCII-8K",
		  "ON/OFF:  Internal ESE-MegaSCC+    ON/ON:  Internal ESE-MegaRAM ASCII-16K", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 12
	{
		VALUE,
		44,15, " RAM Mapper ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b01000000, 0,1, dipMapperStr, 24, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
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
		&(sysInfo5.raw), 0b10000000, 0,1, onOffStr, 24, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
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
		0, 0, 0, 0, NULL, 0, 0,0, NULL, 0, CMDTYPE_NONE,{ 0x00 }, false, 
		{ "Thanks to: @KdL, @Ducasp, and @Cayce-msx",
		  "",
		  "GitHub: https://github.com/nataliapc", (char*)NULL }
	},
	{ LABEL, 3,7,  "This program aims to be a helpful tool for configuring and maintaining" },
	{ LABEL, 3,8,  "MSX++ (OCM-PLD) FPGA-based systems." },
	{ LABEL, 3,10, "Use the F1-F5 or TAB to navigate the different panels, and the cursor keys" },
	{ LABEL, 3,11, "to select the items. Information text will appear at bottom." },
	{ LABEL, 3,12, "To modify an item, you can press SPACE, ENTER, or '+' to increase its value" },
	{ LABEL, 3,13, "or '-' to decrease it." },
	{ LABEL, 3,15, "If you want to suggest improvements, feel free to create an issue at the" },
	{ LABEL, 3,16, "GitHub project page, the link is shown below." },
	// END
	{ END }
};


// ========================================================
// Panel constants

enum {
	PANEL_SYSTEM,
	PANEL_VIDEO,
	PANEL_AUDIO,
	PANEL_DIPS,
	PANEL_HELP
};
static const Panel_t pPanels[] = {
	{ " F1:System ",	3,3, 	11, elemSystem },
	{ " F2:Video ",		14,3, 	10, elemVideo },
	{ " F3:Audio ",		24,3,	10, elemAudio },
	{ " F4:DIP-SW ",	34,3,	11, elemDIPs },
	{ " H:Help ",		63,3,	8, elemHelp },
	{ " Q:Exit ",		71,3,	8, NULL },
	{ NULL }
};


// ========================================================
// Dialogs

Dialog_t dlg_exit = {
	0,0,
	{ "Exit?", NULL },
	{ "  Yes  ", "  No   ", NULL },
	0,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

Dialog_t dlg_reset = {
	0,0,
	{ "Reset requested", "", "Do you want to reboot now?", NULL },
	{ "  Yes  ", "  No   ", NULL },
	1,	//defaultButton
	1,	//cancelButton
	DLG_DEFAULT
};

