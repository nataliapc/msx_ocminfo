/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "globals.h"
#include "ocm_ioports.h"
#include "types.h"
#include "dialogs.h"


// ========================================================
// Defines & externals

#define CUSTOM_SPEED_IDX		3	// Custom CPU speed index (elemSystem[3])

#define SETSMART_X				34
#define SETSMART_Y				19
#define SETSMART_SIZE			12

#define VERTICALOFFSET_DEFAULT	3
#define AUDIOPRESET_DEFAULT		0

// Video output values after mapping
enum {
	VIDEOUTPUT_SVID = 0,
	VIDEOUTPUY_RGB15  = 1,
	VIDEOUTPUT_VGA11 = 2,
	VIDEOUTPUY_VGAPLUS  = 3
};

// Video Mode values
enum {
	VIDEOMODE_PAL = 0,
	VIDEOMODE_AUTO = 1,
	VIDEOMODE_NTSC = 2
};

// Extern variables
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
extern OCM_P4B_SysInfo4_2_t sysInfo4_2;
extern OCM_P4C_SysInfo5_t   sysInfo5;
extern OCM_P4E_Version0_t   pldVers0;
extern OCM_P4F_Version1_t   pldVers1;


// ========================================================
// Custom virtual variables

static uint8_t customCpuClockValue;
static uint8_t customCpuModeValue;
static uint8_t customVideoModeValue = VIDEOMODE_AUTO;
static uint8_t customVideoOutputValue;
static uint8_t customVerticalOffsetValue = VERTICALOFFSET_DEFAULT;
static uint8_t customAudioPresetValue = AUDIOPRESET_DEFAULT;
static uint8_t customLockAllToggles;
static uint8_t customSlots12Value;

static const uint8_t customVideoOutputMap[4] = { 0, 2, 1, 3 };
static const uint8_t customSlots12Map[8] = { 0, 1, 4, 5, 2, 3, 6, 7 };


// ========================================================
// ANCHOR: Text constants

static const char *ocminfoVersionStr = "\x13 ocminfo v"VERSION" \x14";

#define MACHINETYPE_UNKNOWN	 	15		// Unknown machine
static const char *machineTypeStr[16] = {
	"1chipMSX", "Zemmix Neo/SX-1", "SM-X/MC2P", "SX-2", "SM-X Mini/SM-X HB/u2-SX",
	"DE0CV+DEOCM", "SX-E/SX-Lite", "Unknown", "Unknown", "Unknown", "Unknown",
	"Unknown", "Unknown", "Unknown", "Unknown", "Unknown"
};
static const char *sdramSizeStr[4] = {
	"8", "16", "32", "? "
};
static const char *sdramSizeAuxStr[8] = {
	"64", "128", "192", "256", "384", "512", "768", "1024"
};
static const char *onOffStr[2] = {
	"OFF", "ON "
};
static const char *cmtOnOffStr[3] = {
	"OFF", "ON ", "ON "
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
static const char *spriteLimitStr[2] = {
	"4/8", "8/8"
};
static const char *audioPresetStr[7] = {
	"#Custom         ", "#1 Mute Sound   ", "#2 Middle Sound ", "#3 High Sound   ",
	"#4 Emphasis PSG ", "#5 Emphasis SCC+", "#6 Emphasis OPLL"
};
static const char *dipCpuStr[2] = {
	"Standard", "Custom  "
};
static const char *virtualDipVideoStr[4] = {
	"Cmp/S-Vid", "RGB 15KHz", "VGA (1:1)", "VGA+     "	// for Mapped value
};
static const char *dipVideoStr[4] = {
	"Cmp/S-Vid", "VGA (1:1)", "RGB 15KHz", "VGA+     "	// for Raw value
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
	"2048KB", "4096KB"
};


// ========================================================
// ANCHOR: Elements for System Info panel

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
		&customCpuClockValue, 0b00001111, 0,10, cpuClockStr, 18,
		CMDTYPE_NONE,
		{ 0x00 }, 
		false,
		desc_cpuClock,
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,8, " CPU Mode ",
		-1, 1, 7, 7,
		&customCpuModeValue, 0b00000011, 0,2, cpuModeStr, 19,
		CMDTYPE_CUSTOM_CPUMODE, 
		{ OCM_SMART_CPU358MHz, OCM_SMART_TurboPana, OCM_SMART_NullCommand }, 
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE | ATR_SENDSMARTLAST,
		desc_cpuMode,
		IOREV_ALL, M_ALL
	},
	// 3	(referenced by CUSTOM_SPEED_IDX)
	{
		SLIDER,
		3,9, " Custom Speed ",
		-1, 1, 6, 6,
		&(sysInfo0.raw), 0b00000111, 1,7, customSpeedStr, 14,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CPU410MHz, OCM_SMART_CPU410MHz, OCM_SMART_CPU448MHz, OCM_SMART_CPU490MHz, 
		  OCM_SMART_CPU539MHz, OCM_SMART_CPU610MHz, OCM_SMART_CPU696MHz, OCM_SMART_CPU806MHz }, 
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE | ATR_SENDSMARTLAST,
		desc_customSpeed,
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,10, " Ext.Bus Clock ",
		-1, 1, 5, 5,
		&(sysInfo2.raw), 0b00000010, 0,1, extBusStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ExtBusCPU, OCM_SMART_ExtBus358 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_extBusClock,
		IOREV_3, M_ALL
	},
	// 5
	{
		SLIDER,
		3,12, " T. Pana redir. ",
		-1, 1, 6, 6,
		&(sysInfo0.raw), 0b00010000, 0,1, onOffStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_TPanaRedOFF, OCM_SMART_TPanaRedON },
		ATR_SAVEINPROFILE,
		desc_tPanaRedir,
		IOREV_1, M_ALL
	},
	// 6
	{
		SLIDER,
		3,13, " Turbo MegaSD ",
		-1, -5, 5, 5,
		&(sysInfo0.raw), 0b00001000, 0,1, onOffStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_TMegaSDOFF, OCM_SMART_TMegaSDON },
		ATR_SAVEINPROFILE,
		desc_turboMegaSD,
		IOREV_1, M_ALL
	},
	// 7
	{
		LABEL, 
		42,5, "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Keyboard \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 8
	{
		VALUE,
		42,7, " Default Keyboard ",
		3, 1, -7, -7,
		&(pldVers1.raw), 0b10000000, 0,1, keyboardStr, 27,
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		desc_defaultKeyboard,
		IOREV_1, M_ALL
	},
	// 9
	{
		SLIDER,
		42,8, " Current Keyboard ",
		-1, 2, -7, -7,
		&(sysInfo1.raw), 0b00000010, 0,1, keyboardStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_KBLayoutJP, OCM_SMART_KBLayoutNJP },
		ATR_SAVEINPROFILE,
		desc_currentKeyboard,
		IOREV_1, M_ALL
	},
	// 10
	{
		LABEL, 
		42,11, "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 System \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 11
	{
		BUTTON,
		42,13, " Reset to default settings ",
		-2, -3, -5, -5,
		NULL, 0, 0,0, NULL, 0,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ResetDefaults },
		ATR_FORCEPANELRELOAD | ATR_AREYOUSURE,
		desc_restoreDefaults,
		IOREV_1, M_ALL
	},
	// END
	{ END }
};

// ========================================================
// ANCHOR: Elements for Video panel

static const Element_t elemVideo[] = {
	// 0
	{
		SLIDER,
		3,6, " Video Mode ",
		6, 1, 0, 0,
		&customVideoModeValue, 0b00000011, 0,2, videoModeStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ForcePAL, OCM_SMART_VideoAuto, OCM_SMART_ForceNTSC },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_videoMode,
		IOREV_4, M_ALL
	},
	// 1
	{
		SLIDER,
		3,8, " Legacy Output ",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00100000, 0,1, legacyVgaStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_LegacyVGA, OCM_SMART_LegacyVGAplus },
		false,
		desc_legacyOutput,
		IOREV_7, M_ALL
	},
	// 2
	{
		SLIDER,
		3,9, " Scanlines VGA/VGA+ ",
		-1, 1, 0, 0,
		&(sysInfo4_0.raw), 0b00000011, 0,3, scanlinesStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_Scanlines00, OCM_SMART_Scanlines25, OCM_SMART_Scanlines50, OCM_SMART_Scanlines75 },
		ATR_SAVEINPROFILE,
		desc_scanlines,
		IOREV_10, M_SECOND_GEN
	},
	// 3
	{
		SLIDER,
		3,11, " Vertical offset ",
		-1, 1, 0, 0,
		&(customVerticalOffsetValue), 0b00001111, 0,8, verticalOffsetStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VertOffset16, OCM_SMART_VertOffset17, OCM_SMART_VertOffset18,
		  OCM_SMART_VertOffset19, OCM_SMART_VertOffset20, OCM_SMART_VertOffset21,
		  OCM_SMART_VertOffset22, OCM_SMART_VertOffset23, OCM_SMART_VertOffset24 },
		ATR_SAVEINPROFILE,
		desc_verticalOffset,
		IOREV_8, M_ALL
	},
	// 4
	{
		SLIDER,
		3,13, " VDP Speed ",
		-1, 1, 0, 0,
		&(sysInfo0.raw), 0b00100000, 0,1, vdpSpeedStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VDPNormal, OCM_SMART_VDPFast },
		ATR_SAVEINPROFILE,
		desc_vdpSpeed,
		IOREV_2, M_ALL
	},
	// 5
	{
		SLIDER,
		3,14, " Center YJK modes ",
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00010000, 0,1, onOffStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CenterYJKOFF, OCM_SMART_CenterYJKON },
		ATR_SAVEINPROFILE,
		desc_centerYJKmodes,
		IOREV_7, M_ALL
	},
	// 6
	{
		SLIDER,
		3,15, " Sprite Limit ",
		-1, -6, 0, 0,
		&(sysInfo4_2.raw), 0b00000001, 0,1, spriteLimitStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_SpriteLimit48, OCM_SMART_SpriteLimit88 },
		ATR_SAVEINPROFILE,
		desc_spriteLimit,
		IOREV_12, M_ALL
	},
	// END
	{ END }
};

// ========================================================
// ANCHOR: Elements for Audio panel

static const Element_t elemAudio[] = {
	// 0
	{
		VALUE,
		3,6, " Audio presets ",
		7, 1, 8, 8,
		&customAudioPresetValue, 0b00000111, 0,6, audioPresetStr, 14,
		CMDTYPE_STANDARD,
		{ OCM_SMART_NullCommand, OCM_SMART_AudioPreset1, OCM_SMART_AudioPreset2, OCM_SMART_AudioPreset3,
		  OCM_SMART_AudioPreset4, OCM_SMART_AudioPreset5, OCM_SMART_AudioPreset6 },
		ATR_FORCEPANELRELOAD,
		desc_audioPresets,
		IOREV_7, M_ALL
	},
	// 1
	{
		CUSTOM_VOLUME_SLIDER,
		3,8, " Master volume ",
		-1, 1, 8, 8,
		&(audioVols0.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_MasterVol0, OCM_SMART_MasterVol1, OCM_SMART_MasterVol2, OCM_SMART_MasterVol3,
		  OCM_SMART_MasterVol4, OCM_SMART_MasterVol5, OCM_SMART_MasterVol6, OCM_SMART_MasterVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_masterVolume,
		IOREV_4, M_ALL
	},
	// 2
	{
		CUSTOM_VOLUME_SLIDER,
		3,9, " PSG volume ",
		-1, 1, 7, 7,
		&(audioVols0.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PSGVol0, OCM_SMART_PSGVol1, OCM_SMART_PSGVol2, OCM_SMART_PSGVol3,
		  OCM_SMART_PSGVol4, OCM_SMART_PSGVol5, OCM_SMART_PSGVol6, OCM_SMART_PSGVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_psgVolume,
		IOREV_4, M_ALL
	},
	// 3
	{
		CUSTOM_VOLUME_SLIDER,
		3,10, " SCC+ volume ",
		-1, 1, 6, 6,
		&(audioVols1.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_SCCIVol0, OCM_SMART_SCCIVol1, OCM_SMART_SCCIVol2, OCM_SMART_SCCIVol3,
		  OCM_SMART_SCCIVol4, OCM_SMART_SCCIVol5, OCM_SMART_SCCIVol6, OCM_SMART_SCCIVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_sccVolume,
		IOREV_4, M_ALL
	},
	// 4
	{
		CUSTOM_VOLUME_SLIDER,
		3,11, " OPLL volume ",
		-1, 1, 5, 5,
		&(audioVols1.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPLLVol0, OCM_SMART_OPLLVol1, OCM_SMART_OPLLVol2, OCM_SMART_OPLLVol3, 
		  OCM_SMART_OPLLVol4, OCM_SMART_OPLLVol5, OCM_SMART_OPLLVol6, OCM_SMART_OPLLVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_opllVolume,
		IOREV_4, M_ALL
	},
	// 5
	{
		SLIDER,
		3,13, " PSG2 ",
		-1, 1, 4, 4,
		&(sysInfo4_0.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_IntPSG2OFF, OCM_SMART_IntPSG2ON },
		ATR_SAVEINPROFILE,
		desc_psg2,
		IOREV_11, M_SECOND_GEN
	},
	// 6
	{
		SLIDER,
		3,15, " OPL3 ",
		-1, 1, 3, 3,
		&(sysInfo1.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPL3OFF, OCM_SMART_OPL3ON },
		ATR_SAVEINPROFILE,
		desc_opl3,
		IOREV_10, M_SECOND_GEN
	},
	// 7
	{
		SLIDER,
		3,17, " CMT I/F ",
		-1, -7, 2, 2,
		&(sysInfo1.raw), 0b00000100, 0,1, cmtOnOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CMTOFF, OCM_SMART_CMTON },
		ATR_SAVEINPROFILE|ATR_USELASTSTRFORNA,
		desc_cmtIF,
		IOREV_ALL, M_FIRST_GEN
	},
	// 8
	{
		SLIDER,
		40,6, " Pseudo stereo ",
		1, 1, -8, -8,
		&(sysInfo2.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PseudSterOFF, OCM_SMART_PseudSterON },
		ATR_SAVEINPROFILE,
		desc_pseudoStereo,
		IOREV_3, M_ALL
	},
	// 9
	{
		SLIDER,
		40,8, " Right Inverse Audio ",
		-1, -1, -8, -8,
		&(sysInfo3.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_RightInvAud0, OCM_SMART_RightInvAud1 },
		ATR_SAVEINPROFILE,
		desc_rightInverseAudio,
		IOREV_5, M_ALL
	},
	// END
	{ END }
};

// ========================================================
// ANCHOR: Elements for DIPs panels

static const Element_t elemDIPs[] = {
	// 0
	{
		LABEL, 
		3,5,  "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Virtual DIPs \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 1
	{
		SLIDER,
		3,7, " CPU Clock ",
		5, 1, 7, 7,
		&(virtualDIPs.raw), 0b00000001, 0,1, dipCpuStr, 20, 
		CMDTYPE_CUSTOM_CPUMODE,
		{ OCM_SMART_CPU358MHz, OCM_SMART_NullCommand },
		ATR_FORCEPANELRELOAD,
		desc_vCpuClock,
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,9, " Video Output ",
		-1, 1, 7, 7,
		&customVideoOutputValue, 0b00000011, 0,3, virtualDipVideoStr, 18, 
		CMDTYPE_STANDARD,
		{ OCM_SMART_Disp15KhSvid, OCM_SMART_Disp15KhRGB, OCM_SMART_Disp31KhVGA, OCM_SMART_Disp31KhVGAp },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_vVideoOutput,
		IOREV_ALL, M_ALL
	},
	// 3
	{
		SLIDER,
		3,11, " Cartridge Slot 1 ",
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000001, 0,1, dipSlot1Str, 20, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
		  OCM_SMART_S1extS2a16, OCM_SMART_S1sccS2a16 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_vCartridgeSlot1,
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,13, " Cartridge Slot 2 ",
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000110, 0,3, virtualDipSlot2Str, 18, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
		  OCM_SMART_S1extS2a16, OCM_SMART_S1sccS2a16 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_vCartridgeSlot2,
		IOREV_ALL, M_ALL
	},
	// 5
	{
		VALUE,
		3,15, " RAM Mapper ",
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b01000000, 0,1, dipMapperStr, 27,
		CMDTYPE_NONE,
/*REV*/	/*{ OCM_SMART_Mapper4MbOFF, OCM_SMART_Mapper4MbON },
		ATR_FORCEPANELRELOAD | ATR_NEEDRESETTOAPPLY,*/ {0x00}, false,
		desc_vRamMapper,
		IOREV_ALL, M_ALL
	},
	// 6
	{
		VALUE,
		3,17, " Internal MegaSD ",
		-1, -5, 7, 7,
		&(virtualDIPs.raw), 0b10000000, 0,1, onOffStr, 27,
		CMDTYPE_NONE,
/*REV*/	/*{ OCM_SMART_MegaSDOFF, OCM_SMART_MegaSDON },
		ATR_FORCEPANELRELOAD | ATR_NEEDRESETTOAPPLY,*/ {0x00}, false,
		desc_vInternalMegaSD,
		IOREV_ALL, M_ALL
	},
	// 7
	{
		LABEL, 
		42,5, "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17\x13 Hardware DIPs \x14\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
	},
	// 8
	{
		VALUE,
		42,7, " CPU Clock ",
		5, 1, -7, -7,
		&(sysInfo5.raw), 0b00000001, 0,1, dipCpuStr, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		desc_hCpuClock,
		IOREV_5, M_ALL
	},
	// 9
	{
		VALUE,
		42,9, " Video Output ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00000110, 0,3, dipVideoStr, 20, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		desc_hVideoOutput,
		IOREV_5, M_ALL
	},
	// 10
	{
		VALUE,
		42,11, " Cartridge Slot 1 ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00001000, 0,1, dipSlot1Str, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		desc_hCartridgeSlot1,
		IOREV_5, M_ALL
	},
	// 11
	{
		VALUE,
		42,13, " Cartridge Slot 2 ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00110000, 0,3, dipSlot2Str, 20, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		desc_hCartridgeSlot2,
		IOREV_5, M_ALL
	},
	// 12
	{
		VALUE,
		42,15, " RAM Mapper ",
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b01000000, 0,1, dipMapperStr, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		desc_hRamMapper,
		IOREV_5, M_ALL
	},
	// 13
	{
		VALUE,
		42,17, " Internal MegaSD ",
		-1, -5, -7, -7,
		&(sysInfo5.raw), 0b10000000, 0,1, onOffStr, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		desc_hInternalMegaSD,
		IOREV_5, M_ALL
	},
	// END
	{ END }
};

// ========================================================
// ANCHOR: Elements for locks panel

static const Element_t elemLocks[] = {
	// 0
	{
		SLIDER,
		3,6, " Lock all key toggles ",
		4, 1, 0, 0,
		&customLockAllToggles, 0b00000001, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockAll, OCM_SMART_LockAll },
		ATR_FORCEPANELRELOAD,
		desc_lockAllToggles,
		IOREV_ALL, M_ALL
	},
	// 1
	{
		SLIDER,
		3,9, " Lock CPU Mode ",
		-1, 1, 4, 4,
		&(lockToggles.raw), 0b00000001, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockTurbo, OCM_SMART_LockTurbo },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockCpuMode,
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,11, " Lock Video Output ",
		-1, 1, 4, 4,
		&(lockToggles.raw), 0b00000010, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockDisplay, OCM_SMART_LockDisplay },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockVideoOutput,
		IOREV_ALL, M_ALL
	},
	// 3
	{
		SLIDER,
		3,13, " Lock Audio Mixer ",
		-1, 1, 4, 4,
		&(lockToggles.raw), 0b00000100, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockAudio, OCM_SMART_LockAudio },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockAudioMixer,
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,15, " Lock Reset key ",
		-1, -4, 4, 4,
		&(lockToggles.raw), 0b00100000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockHardRst, OCM_SMART_LockHardRst },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockResetKey,
		IOREV_ALL, M_ALL
	},
	// 5
	{
		SLIDER,
		42,9, " Lock Slot-1 ",
		3, 1, -4, -4,
		&(lockToggles.raw), 0b00001000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockSlot1, OCM_SMART_LockSlot1 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockSlot1,
		IOREV_ALL, M_ALL
	},
	// 6
	{
		SLIDER,
		42,11, " Lock Slot-2 ",
		-1, 1, -4, -4,
		&(lockToggles.raw), 0b00010000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockSlot2, OCM_SMART_LockSlot2 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockSlot2,
		IOREV_ALL, M_ALL
	},
	// 7
	{
		SLIDER,
		42,13, " Lock Internal Mapper ",
		-1, 1, -4, -4,
		&(lockToggles.raw), 0b01000000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockMapper, OCM_SMART_LockMapper },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockInternalMapper,
		IOREV_ALL, M_ALL
	},
	// 8
	{
		SLIDER,
		42,15, " Lock Internal MegaSD ",
		-1, -3, -4, -4,
		&(lockToggles.raw), 0b10000000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockMegaSD, OCM_SMART_LockMegaSD },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		desc_lockInternalMegaSD,
		IOREV_ALL, M_ALL
	},
	// END
	{ END }
};

// ========================================================
// ANCHOR: Elements for Help panel

static const Element_t elemHelp[] = {
	{ LABEL, 3,5,  "    --==[ OCMINFO conio.lib dos.lib ]==--  by "AUTHOR"    ",
		0, 0, 0, 0, NULL, 0, 0,0, NULL, 0, CMDTYPE_NONE,{ 0x00 }, false, 
		desc_help
	},
	{ LABEL, 3,7,  "This program aims to be a helpful tool for configuring and maintaining" },
	{ LABEL, 3,8,  "MSX++ (OCM-PLD) FPGA-based systems." },
	{ LABEL, 3,10, "Use the F1-F5 or TAB to navigate the different panels, and the cursor keys" },
	{ LABEL, 3,11, "to select the items. Description text will appear at bottom." },
	{ LABEL, 3,12, "To modify a value, you can press SPACE, RETURN, '+' to increase it or press" },
	{ LABEL, 3,13, "BS, '-' to decrease it." },

	{ LABEL, 3,15, "Note: descriptions are related to the latest I/O specifications released." },	

	{ LABEL, 3,17, "If you want to suggest improvements, feel free to create an issue at the" },
	{ LABEL, 3,18, "GitHub project page, the link is shown below." },
	// END
	{ END }
};


// ========================================================
// ANCHOR: String descriptions

const char *desc_cpuClock[] = {
	"Currently active CPU clock frequency (read only).",
	"Customize below values to adjust it.",
	NULL
};
const char *desc_cpuMode[] = {
	"Standard: Standard CPU clock mode (3.58MHz)",
	"tPANA:    Panasonic turbo mode (aka Turbo Pana, 5.37MHz)",
	"Custom:   Custom CPU speed mode (4.10MHz to 8.06MHz)"
};
const char *desc_customSpeed[] = {
	"Custom CPU speed. These are not real CPU frequencies but a simulation",
	"slowing down the 10.74MHz frequency. The values have been calculated and",
	"are purely indicative. The default is 8.06MHz."
};
const char *desc_extBusClock[] = {
	"Synchronize external bus clock to CPU clock (default), or force it to",
	"3.58MHz. In async mode, tPANA is the default turbo clock, and the F12 key is",
	"restricted to protect external cartridges."
};
const char *desc_tPanaRedir[] = {
	"Turbo Pana redirect mode sets Turbo Pana I/O to the current Custom Speed.",
	"This allows games to reach 8.06MHz or other frequencies using port 41h ID8,",
	"making them OCM-compatible without software patch."
};
const char *desc_turboMegaSD[] = {
	"Turbo MegaSD sets SD card speed access at loading time activating 8.06MHz,",
	"so you get fast load even with CPU at 3.58/5.37MHz. It can adversely affect",
	"external cartridges that do not support 8.06MHz."
};
const char *desc_defaultKeyboard[] = {
	"Default keyboard layout (read only).",
	"Values can be Japanese, or non-Japanese (a core layout variant, currently",
	"available are BR, ES, FR, IT, US)."
};
const char *desc_currentKeyboard[] = {
	"Currently active keyboard layout.",
	"Values can be Japanese, or non-Japanese (a core layout variant, currently",
	"available are BR, ES, FR, IT, US)."
};
const char *desc_restoreDefaults[] = {
	"Pressing this button will reset all settings to their default values.",
	"All current user-changed settings will be lost.",
	"Profiles will not be affected."
};

const char *desc_videoMode[] = {
	"Video output mode (Auto/PAL/NTSC)",
	"Auto:     Set to auto (default) that is bound by VDP Control Register #9.",
	"PAL/NTSC: Force video output to PAL(50Hz) or NTSC(60Hz)."
};
const char *desc_legacyOutput[] = {
	"Assign Legacy Output to VGA or VGA+ (default).",
	NULL
};
const char *desc_scanlines[] = {
	"Visualization of scanlines for VGA/VGA+ (mainly for 2nd Gen machines).",
	NULL
};
const char *desc_verticalOffset[] = {
	"Changes the vertical offset.",
	"Default is 19; value 16 is useful for Ark-A-Noah; value 24 is useful",
	"for Space Manbow. Modify it only in special cases, not permanently."
};
const char *desc_vdpSpeed[] = {
	"VDP Speed: Normal (default) or Fast (V9958 only).",
	"Normal: Works like real hardware.",
	"Fast:   The VDP works faster."
};
const char *desc_centerYJKmodes[] = {
	"Allows forcing the centering of YJK modes and VDP R#25 mask, useful for",
	"MSX2+ games. Default is OFF.",
	"Enable it only in special cases, not permanently."
};
const char *desc_spriteLimit[] = {
	"Sets sprite limit per line.",
	"4/8: Standard mode. 4 for MSX1 screens, 8 for MSX2 and above (default).",
	"8/8: Enhanced mode. Force MSX1 screens to allow 8 sprites per line."
};

const char *desc_audioPresets[] = {
	"Set audio presets (press +/- to change):",
	"#Custom preset, #1:Mute, #2:Middle Sound, #3:High Sound (default option),",
	"#4:Emphasis PSG Sound, #5:Emphasis SCC+ Sound, #6:Emphasis OPLL Sound."
};
const char *desc_masterVolume[] = {
	"Set Master volume level.",
	"Default is 7.",
	NULL
};
const char *desc_psgVolume[] = {
	"Set PSG volume level.",
	"Default is 4.",
	NULL
};
const char *desc_sccVolume[] = {
	"Set SCC+ volume level.",
	"Default is 4.",
	NULL
};
const char *desc_opllVolume[] = {
	"Set OPLL volume level.",
	"Default is 4.",
	NULL
};
const char *desc_psg2[] = {
	"Enable/disable an additional internal PSG (acting as an external PSG).",
	"Default is OFF.",
	NULL
};
const char *desc_opl3[] = {
	"Enable/disable the OPL3 sound chipset.",
	"Default is OFF.",
	NULL
};
const char *desc_cmtIF[] = {
	"Cassette tape interface:",
	"On 1st Gen machines CMT is modifiable. Default is OFF.",
	"On 2nd Gen machines CMT is always enabled."
};
const char *desc_pseudoStereo[] = {
	"Enable/disable the Pseudo-Stereo mode (needs an external sound cartridge).",
	"Default is OFF.",
	NULL
};
const char *desc_rightInverseAudio[] = {
	"Enable/disable the Right Inverse Audio, which is a good solution for",
	"recording really clean balanced audio.",
	"Default is OFF."
};

const char *desc_vCpuClock[] = {
	"Virtual DIP-Switch #1: CPU Clock",
	"OFF: Standard CPU clock mode (3.58MHz)",
	"ON:  Custom CPU speed mode (4.10MHz to 8.06MHz)"
};
const char *desc_hCpuClock[] = {
	"Hardware DIP-Switch #1: CPU Clock",
	"OFF: Standard CPU clock mode (3.58MHz)",
	"ON:  Custom CPU speed mode (4.10MHz to 8.06MHz)"
};
const char *desc_vVideoOutput[] = {
	"Virtual DIP-Switch #2-#3: Video Output",
	"OFF/OFF: Composite/S-Video w/mono audio     OFF/ON: RGB 15KHz",
	"ON/OFF:  VGA Mode w/Pixel 1:1               ON/ON:  VGA+ Mode for CRT"
};
const char *desc_hVideoOutput[] = {
	"Hardware DIP-Switch #2-#3: Video Output",
	"OFF/OFF: Composite/S-Video w/mono audio     OFF/ON: RGB 15KHz",
	"ON/OFF:  VGA Mode w/Pixel 1:1               ON/ON:  VGA+ Mode for CRT"
};
const char *desc_vCartridgeSlot1[] = {
	"Virtual DIP-Switch #4: Cartridge Slot 1 Configuration",
	"OFF: External Slot-1 / Optional Slot-3 (shared)",
	"ON:  Internal ESE-MegaSCC+"
};
const char *desc_hCartridgeSlot1[] = {
	"Hardware DIP-Switch #4: Cartridge Slot 1 Configuration",
	"OFF: External Slot-1 / Optional Slot-3 (shared)",
	"ON:  Internal ESE-MegaSCC+"
};
const char *desc_vCartridgeSlot2[] = {
	"Virtual DIP-Switch #5-#6: Cartridge Slot 2 Configuration",
	"OFF/OFF: External Slot-2          OFF/ON: Internal ESE-MegaRAM ASCII-8K",
	"ON/OFF:  Internal ESE-MegaSCC+    ON/ON:  Internal ESE-MegaRAM ASCII-16K"
};
const char *desc_hCartridgeSlot2[] = {
	"Hardware DIP-Switch #5-#6: Cartridge Slot 2 Configuration",
	"OFF/OFF: External Slot-2          OFF/ON: Internal ESE-MegaRAM ASCII-8K",
	"ON/OFF:  Internal ESE-MegaSCC+    ON/ON:  Internal ESE-MegaRAM ASCII-16K"
};
const char *desc_vRamMapper[] = {
	"Virtual DIP-Switch #7: RAM Mapper",
	"OFF: Internal 2048KB RAM / 1st EPBIOS",
	"ON:  Internal 4096KB RAM / Optional 2nd EPBIOS"
};
const char *desc_hRamMapper[] = {
	"Hardware DIP-Switch #7: RAM Mapper",
	"OFF: Internal 2048KB RAM / 1st EPBIOS",
	"ON:  Internal 4096KB RAM / Optional 2nd EPBIOS"
};
const char *desc_vInternalMegaSD[] = {
	"Virtual DIP-Switch #8: SD card Slot",
	"OFF: Disabled",
	"ON:  Enabled"
};
const char *desc_hInternalMegaSD[] = {
	"Hardware DIP-Switch #8: SD card Slot",
	"OFF: Disabled",
	"ON:  Enabled"
};

const char *desc_lockAllToggles[] = {
	"Locks all key toggles (ON/OFF). Locks are designed to allow programs to",
	"interact with Smart Commands without interferences of configuration keys",
	"pressed by user. Default is OFF.",
	NULL
};

const char *desc_lockCpuMode[] = {
	"Locks CPU Clock changes when pressing F12 changing DIP-SW1.",
	"Default is OFF.",
	NULL
};

const char *desc_lockVideoOutput[] = {
	"Locks Video Output changes when pressing (SHIFT+)PRTSCR changing",
	"DIP-SW2/3.",
	"Default is OFF."
};

const char *desc_lockAudioMixer[] = {
	"Locks Audio changes when pressing (SHIFT+)PGUP/PGDOWN/F9/F10/F11.",
	"CMT I/F and OPL3 are also affected when pressing SCRLK key.",
	"Default is OFF."
};

const char *desc_lockResetKey[] = {
	"Locks reset keys/buttons: when pressing LCTRL+F12 for Cold Reset, ",
	"LCTRL+SHIFT+F12 for Full Reset, and machine System Reset button.",
	"Default is OFF."
};

const char *desc_lockSlot1[] = {
	"Locks Slot-1 changes when pressing SHIFT+F12 changing DIP-SW4.",
	"Default is OFF.",
	NULL
};

const char *desc_lockSlot2[] = {
	"Locks Slot-2 changes when pressing SHIFT+SCRLK changing DIP-SW5/6.",
	"Default is OFF.",
	NULL
};

const char *desc_lockInternalMapper[] = {
	"Locks Internal Mapper when changing DIP-SW7.",
	"Default is OFF.",
	NULL
};

const char *desc_lockInternalMegaSD[] = {
	"Locks Internal MegaSD when changing DIP-SW8.",
	"Default is OFF.",
	NULL
};

const char *desc_help[] = {
	"Thanks to: @KdL, @HRA!, @Ducasp, and @Cayce-msx",
	"",
	"GitHub: https://github.com/nataliapc"
};


// ========================================================
// Panel constants

enum {
	PANEL_SYSTEM = 0,
	PANEL_VIDEO,
	PANEL_AUDIO,
	PANEL_DIPS,
	PANEL_LOCKS,
	PANEL_HELP,
	PANEL_PROFILES,
	PANEL_EXIT
};
#define PANEL_FIRST		PANEL_SYSTEM
#define PANEL_LAST		PANEL_LOCKS

static const Panel_t pPanels[] = {
	{ " F1:System ",	2,3, 	11,	elemSystem },
	{ " F2:Video ",		12,3, 	10,	elemVideo },
	{ " F3:Audio ",		21,3,	10,	elemAudio },
	{ " F4:DIP-SW ",	30,3,	11,	elemDIPs },
	{ " F5:Locks ",		40,3,	10,	elemLocks },
	{ " [A]bout ",		53,3,	9,	elemHelp },
	{ " [P]rofiles ",	61,3,	12,	NULL },
	{ " E[x]it ",		72,3,	8,	NULL },
	{ NULL }
};


// ========================================================
// Dialogs

const char *dlg_exitStr[] = {
	" Exit? ", NULL
};
const Dialog_t dlg_exit = {
	0,0,
	dlg_exitStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_NO,		//cancelButton
	DLG_DEFAULT
};

const char *dlg_resetStr[] = {
	"Reset requested", "", "Do you want to reboot now?", NULL
};
const Dialog_t dlg_reset = {
	0,0,
	dlg_resetStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_YES,	//cancelButton
	DLG_DEFAULT
};

const char *dlg_confirmStr[] = {
	"Confirm", "", "Are you sure?", NULL
};
const Dialog_t dlg_confirm = {
	0,0,
	dlg_confirmStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_NO,		//cancelButton
	DLG_DEFAULT
};
