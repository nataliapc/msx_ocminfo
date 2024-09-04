/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ocm_ioports.h"
#include "types.h"
#include "dialogs.h"


// ========================================================
// Defines & externals

#define VERSION		"0.9.5b3"

#define SETSMART_X		34
#define SETSMART_Y		19
#define SETSMART_SIZE	12

extern OCM_P42_VirtualDIP_t virtualDIPs;
extern OCM_P43_LockToggles_t lockToggles;
extern OCM_P44_LedLights_t  ledLights;
extern OCM_P45_AudioVol0_t  audioVols0;
extern OCM_P46_AudioVol1_t  audioVols1;
extern OCM_P47_SysInfo0_t   sysInfo0;
extern OCM_P48_SysInfo1_t   sysInfo1;
extern OCM_P49_SysInfo2_t   sysInfo2;
extern OCM_P4A_SysInfo3_t   sysInfo3;
extern OCM_P4B_SysInfo4_0_t sysInfo4_0;
extern OCM_P4B_SysInfo4_1_t sysInfo4_1;
extern OCM_P4C_SysInfo5_t   sysInfo5;
extern OCM_P4E_Version0_t   pldVers0;
extern OCM_P4F_Version1_t   pldVers1;


// ========================================================
// Custom virtual variables

static uint8_t customCpuClockValue;
static uint8_t customCpuModeValue;
static uint8_t customVideoModeValue;
static uint8_t customVideoOutputValue;
static uint8_t customVerticalOffsetValue = 3;
static uint8_t customAudioPresetValue = 0;
static uint8_t customSlots12Value;

static const uint8_t customVideoOutputMap[4] = { 0, 2, 1, 3 };
static const uint8_t customSlots12Map[8] = { 0, 1, 4, 5, 2, 3, 6, 7 };


// ========================================================
// Text constants

static const char *ocminfoVersionStr = "\x13 ocminfo v"VERSION" \x14";

static const char *machineTypeStr[16] = {
	"1ChipMSX", "Zemmix Neo/SX-1", "SM-X/MCP2", "SX-2", "SM-X Mini/SM-X HB", 
	"DE0CV", "??", "??", "??", "??", "??", "??", "??", "??", "??", "Unknown"
};
static const char *sdramSizeStr[4] = {
	"8", "16", "32", "--"
};
static const char *sdramSizeAuxStr[8] = {
	"64", "128", "192", "256", "320", "384", "448", "512"
};
static const char *onOffStr[2] = {
	"OFF", "ON "
};
static const char *numbersStr[8] = {
	"0", "1", "2", "3", "4", "5", "6", "7"
};
static const char *cpuClockStr[9] = {
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
static const char *vdpSpeedStr[2] = {
	"Normal", "Fast  "
};
static const char *videoModeStr[3] = {
	"PAL(50Hz) ", "Auto      ", "NTSC(60Hz)"
};
static const char *legacyVgaStr[2] = {
	"VGA ", "VGA+"
};
static const char *verticalOffsetStr[9] = {
	"16", "17", "18", "19", "20", "21", "22", "23", "24"
};
static const char *scanlinesStr[4] = {
	"0% ", "25%", "50%", "75%"
};
static const char *audioPresetStr[7] = {
	"#Custom         ", "#1 Mute Sound   ", "#2 Middle Sound ", "#3 High Sound   ",
	"#4 Emphasis PSG ", "#5 Emphasis SCC+", "#6 Emphasis OPLL"
};
static const char *dipCpuStr[2] = {
	"Standard", "Custom  "
};
static const char *virtualDipVideoStr[4] = {
	"Cmp/S-Vid", "RGB 15KHz", "VGA 1:1  ", "VGA+     "
};
static const char *dipVideoStr[4] = {
	"Cmp/S-Vid", "VGA 1:1  ", "RGB 15KHz", "VGA+     "
};
static const char *dipSlot1Str[2] = {
	"External", "Int.SCC+"
};
static const char *virtualDipSlot2Str[4] = {
	"External", "Int.A8K ", "Int.SCC+", "Int.A16K"		// for Mapped value
};
static const char *dipSlot2Str[4] = {
	"External", "Int.SCC+", "Int.A8K ", "Int.A16K"		// for Raw value
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
		CUSTOM_CPUCLOCK_VALUE,
		3,7, " CPU Clock ",
		5, 1, 7, 7,
		&customCpuClockValue, 0b00001111, 0,10, cpuClockStr, 28,
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
		{ "Standard: Standard CPU clock mode (3.58MHz)",
		  "tPANA:    Panasonic turbo mode (aka Turbo Pana, 5.37MHz)",
		  "Custom:   Custom CPU clock mode (4.10MHz to 8.06MHz)", (char*)NULL },
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
		{ "Custom CPU speed. These are not real CPU frequencies but a simulation",
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
		{ "Synchronize external bus clock to CPU clock (default), or force it to",
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
		{ "Turbo Pana redirect mode sets Turbo Pana I/O to the current Custom Speed.",
		  "This allows games to reach 8.06MHz or other frequencies using port 41h ID8,",
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
		{ "Turbo MegaSD sets SD card speed access at loading time activating 8.06MHz,",
		  "so you get fast load even with CPU at 3.58/5.37MHz. It can adversely affect",
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
		3,6, " VDP Speed ",
		5, 1, 0, 0,
		&(sysInfo0.raw), 0b00100000, 0,1, vdpSpeedStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VDPNormal, OCM_SMART_VDPFast },
		false,
		{ "Normal: Works like real hardware (default).",
		  "Fast:   The VDP works faster (V9958 only).", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 1
	{
		SLIDER,
		3,8, " Video Mode ",
		-1, 1, 0, 0,
		&customVideoModeValue, 0b00000011, 0,2, videoModeStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ForcePAL, OCM_SMART_VideoAuto, OCM_SMART_ForceNTSC },
		ATR_FORCEPANELRELOAD,
		{ "Video output mode (Auto/PAL/NTSC)", 
		  "Auto:     Set to auto (default) that is bound by VDP Control Register #9.",
		  "PAL/NTSC: Force video output to PAL(50Hz) or NTSC(60Hz).", (char*)NULL },
		IOREV_1, M_ALL
	},
	// 2
	{
		SLIDER,
		3,10, " Legacy Output ",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00100000, 0,1, legacyVgaStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_LegacyVGA, OCM_SMART_LegacyVGAplus },
		false,
		{ "Assign Legacy Output to VGA or VGA+ (default).", (char*)NULL },
		IOREV_8, M_ALL
	},
	// 3
	{
		SLIDER,
		3,12, " Vertical offset ",
		-1, 1, 0, 0,
		&(customVerticalOffsetValue), 0b00001111, 0,8, verticalOffsetStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VertOffset16, OCM_SMART_VertOffset17, OCM_SMART_VertOffset18,
		  OCM_SMART_VertOffset19, OCM_SMART_VertOffset20, OCM_SMART_VertOffset21,
		  OCM_SMART_VertOffset22, OCM_SMART_VertOffset23, OCM_SMART_VertOffset24 },
		false,
		{ "Changes the vertical offset.",
		  "Default is 19; value 16 is useful for Ark-A-Noah; value 24 is useful",
		  "for Space Manbow.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,14, " Center YJK modes ",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00010000, 0,1, onOffStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CenterYJKOFF, OCM_SMART_CenterYJKON },
		false,
		{ "Allows forcing the centering of YJK modes and VDP R#25 mask, useful for",
		  "MSX2+ games. Default is OFF.", (char*)NULL },
		IOREV_7, M_ALL
	},
	// 5
	{
		SLIDER,
		3,16, " Scanlines VGA/VGA+ ",
		-1, -5, 0, 0,
		&(sysInfo4_0.raw), 0b00000011, 0,3, scanlinesStr, 23,
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
		VALUE,
		3,6, " Audio presets ",
		6, 1, 7, 7,
		&customAudioPresetValue, 0b00000111, 0,6, audioPresetStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_NullCommand, OCM_SMART_AudioPreset1, OCM_SMART_AudioPreset2, OCM_SMART_AudioPreset3,
		  OCM_SMART_AudioPreset4, OCM_SMART_AudioPreset5, OCM_SMART_AudioPreset6 },
		ATR_FORCEPANELRELOAD,
		{ "Set audio presets (press +/- to change):",
		  "#Custom preset, #1:Mute, #2:Middle Sound, #3:High Sound (default option),",
		  "#4:Emphasis PSG Sound, #5:Emphasis SCC+ Sound, #6:Emphasis OPLL Sound.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 1
	{
		CUSTOM_VOLUME_SLIDER,
		3,8, " Master volume ",
		-1, 1, 7, 7,
		&(audioVols0.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_MasterVol0, OCM_SMART_MasterVol1, OCM_SMART_MasterVol2, OCM_SMART_MasterVol3,
		  OCM_SMART_MasterVol4, OCM_SMART_MasterVol5, OCM_SMART_MasterVol6, OCM_SMART_MasterVol7 },
		ATR_FORCEPANELRELOAD,
		{ "Set Master volume level.",
		  "Default is 7.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		CUSTOM_VOLUME_SLIDER,
		3,9, " PSG volume ",
		-1, 1, 6, 6,
		&(audioVols0.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PSGVol0, OCM_SMART_PSGVol1, OCM_SMART_PSGVol2, OCM_SMART_PSGVol3,
		  OCM_SMART_PSGVol4, OCM_SMART_PSGVol5, OCM_SMART_PSGVol6, OCM_SMART_PSGVol7 },
		ATR_FORCEPANELRELOAD,
		{ "Set PSG volume level.",
		  "Default is 4.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 3
	{
		CUSTOM_VOLUME_SLIDER,
		3,10, " SCC+ volume ",
		-1, 1, 5, 5,
		&(audioVols1.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_SCCIVol0, OCM_SMART_SCCIVol1, OCM_SMART_SCCIVol2, OCM_SMART_SCCIVol3,
		  OCM_SMART_SCCIVol4, OCM_SMART_SCCIVol5, OCM_SMART_SCCIVol6, OCM_SMART_SCCIVol7 },
		ATR_FORCEPANELRELOAD,
		{ "Set SCC+ volume level.",
		  "Default is 4.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		CUSTOM_VOLUME_SLIDER,
		3,11, " OPLL volume ",
		-1, 1, 4, 4,
		&(audioVols1.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPLLVol0, OCM_SMART_OPLLVol1, OCM_SMART_OPLLVol2, OCM_SMART_OPLLVol3, 
		  OCM_SMART_OPLLVol4, OCM_SMART_OPLLVol5, OCM_SMART_OPLLVol6, OCM_SMART_OPLLVol7 },
		ATR_FORCEPANELRELOAD,
		{ "Set OPLL volume level.",
		  "Default is 4.", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 5
	{
		SLIDER,
		3,13, " PSG2 ",
		-1, 1, 3, 3,
		&(sysInfo4_0.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_IntPSG2OFF, OCM_SMART_IntPSG2ON },
		false,
		{ "Enable/disable an additional internal PSG (acting as an external PSG).",
		  "Default is OFF.", (char*)NULL },
		IOREV_11, M_SX2|M_SMX_MCP2_ID
	},
	// 6
	{
		SLIDER,
		3,15, " OPL3 ",
		-1, -6, 2, 2,
		&(sysInfo1.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPL3OFF, OCM_SMART_OPL3ON },
		false,
		{ "Enable/disable the OPL3 sound chipset.",
		  "Default is OFF.", (char*)NULL },
		IOREV_10, M_SX2|M_SMX_MCP2_ID
	},
	// 7
	{
		SLIDER,
		40,6, " Pseudo stereo ",
		1, 1, -7, -7,
		&(sysInfo2.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PseudSterOFF, OCM_SMART_PseudSterON },
		false,
		{ "Enable/disable the Pseudo-Stereo mode (needs an external sound cartridge).",
		  "Default is OFF.", (char*)NULL },
		IOREV_3, M_ALL
	},
	// 8
	{
		SLIDER,
		40,8, " Right Inverse Audio ",
		-1, -1, -7, -7,
		&(sysInfo3.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_RightInvAud0, OCM_SMART_RightInvAud1 },
		false,
		{ "Enable/disable the Right Inverse Audio, which is a good solution for",
		  "recording really clean balanced audio.",
		  "Default is OFF.", (char*)NULL },
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
		&customVideoOutputValue, 0b00000011, 0,3, virtualDipVideoStr, 20, 
		CMDTYPE_STANDARD,
		{ OCM_SMART_Disp15KhSvid, OCM_SMART_Disp15KhRGB, OCM_SMART_Disp31KhVGA, OCM_SMART_Disp31KhVGAp },
		ATR_FORCEPANELRELOAD,
		{ "Virtual DIP-Switch #2-#3: Video Output",
		  "OFF/OFF: Composite/S-Video w/mono audio     OFF/ON: RGB 15khz",
		  "ON/OFF:  VGA Mode w/Pixel 1:1               ON/ON:  VGA+ Mode for CRT", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 3
	{
		SLIDER,
		3,11, " Cartridge Slot 1 ",
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000001, 0,1, dipSlot1Str, 22, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
		  OCM_SMART_S1extS2a16, OCM_SMART_S1sccS2a16 },
		ATR_FORCEPANELRELOAD,
		{ "Virtual DIP-Switch #4: Cartridge Slot 1 Configuration",
		  "OFF: External Slot-1 / Optional Slot-3 (shared)",
		  "ON:  Internal ESE-MegaSCC+", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,13, " Cartridge Slot 2 ",
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000110, 0,3, virtualDipSlot2Str, 20, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
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
		{ "Virtual DIP-Switch #8: SD card Slot",
		  "OFF: Disabled",
		  "ON:  Enabled", (char*)NULL },
		IOREV_ALL, M_ALL
	},
	// 7
	{
		LABEL, 
		44,5, "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 HW DIPs \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
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
		{ "Hardware DIP-Switch #8: SD card Slot",
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
	PANEL_SYSTEM = 1,
	PANEL_VIDEO,
	PANEL_AUDIO,
	PANEL_DIPS,
	PANEL_HELP
};
#define PANEL_FIRST		PANEL_SYSTEM
#define PANEL_LAST		PANEL_HELP

static const Panel_t pPanels[] = {
	{ NULL },
	{ " F1:System ",	3,3, 	11,	elemSystem },
	{ " F2:Video ",		14,3, 	10,	elemVideo },
	{ " F3:Audio ",		24,3,	10,	elemAudio },
	{ " F4:DIP-SW ",	34,3,	11,	elemDIPs },
	{ " [H]elp ",		51,3,	8,	elemHelp },
	{ " [P]rofiles ",	59,3,	12,	NULL },
	{ " E[x]it ",		71,3,	8,	NULL },
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

