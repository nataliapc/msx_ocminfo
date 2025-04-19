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
#include "strings_index.h"


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

#define MACHINETYPE_UNKNOWN	 	15		// Unknown machine
static const uint16_t machineTypeStr[16] = {
	VALUE_MT_0, VALUE_MT_1, VALUE_MT_2, VALUE_MT_3, VALUE_MT_4,
	VALUE_MT_5, VALUE_MT_6, VALUE_MT_7, VALUE_MT_8, VALUE_MT_9, VALUE_MT_10,
	VALUE_MT_11, VALUE_MT_12, VALUE_MT_13, VALUE_MT_14, VALUE_MT_15
};
static const uint16_t sdramSizeStr[4] = {
	VALUE_SDRAM_8, VALUE_SDRAM_8, VALUE_SDRAM_32, VALUE_SDRAM_UNK
};
static const uint16_t sdramSizeAuxStr[8] = {
	VALUE_SDRAM_AUX_64, VALUE_SDRAM_AUX_128, VALUE_SDRAM_AUX_192, VALUE_SDRAM_AUX_256,
	VALUE_SDRAM_AUX_384, VALUE_SDRAM_AUX_512, VALUE_SDRAM_AUX_768, VALUE_SDRAM_AUX_1024
};
static const uint16_t onOffStr[2] = {
	VALUE_ONOFF_0, VALUE_ONOFF_1
};
static const uint16_t cmtOnOffStr[3] = {
	VALUE_CMTONOFF_0, VALUE_CMTONOFF_1, VALUE_CMTONOFF_2
};
static const uint16_t numbersStr[8] = {
	VALUE_NUM_0, VALUE_NUM_1, VALUE_NUM_2, VALUE_NUM_3, VALUE_NUM_4, VALUE_NUM_5, VALUE_NUM_6, VALUE_NUM_7
};
static const uint16_t cpuClockStr[9] = {
	// Custom clocks
	VALUE_CPUCLK_0, VALUE_CPUCLK_1, VALUE_CPUCLK_2, VALUE_CPUCLK_3, 
	VALUE_CPUCLK_4, VALUE_CPUCLK_5, VALUE_CPUCLK_6,
	// Standard clocks
	VALUE_CPUCLK_7, VALUE_CPUCLK_8
};
static const uint16_t cpuModeStr[9] = {
	VALUE_CPUMODE_0, VALUE_CPUMODE_1, VALUE_CPUMODE_2
};
static const uint16_t customSpeedStr[8] = {
	VALUE_CUSTSPD_0, VALUE_CUSTSPD_1, VALUE_CUSTSPD_2, VALUE_CUSTSPD_3, 
	VALUE_CUSTSPD_4, VALUE_CUSTSPD_5, VALUE_CUSTSPD_6, VALUE_CUSTSPD_7
};
static const uint16_t extBusStr[2] = {
	VALUE_EXTBUS_0, VALUE_EXTBUS_1
};
static const uint16_t keyboardStr[2] = {
	VALUE_KBD_0, VALUE_KBD_1
};
static const uint16_t vdpSpeedStr[2] = {
	VALUE_VDPSPD_0, VALUE_VDPSPD_1
};
static const uint16_t videoModeStr[3] = {
	VALUE_VIDMODE_0, VALUE_VIDMODE_1, VALUE_VIDMODE_2
};
static const uint16_t legacyVgaStr[2] = {
	VALUE_LEGACYVGA_0, VALUE_LEGACYVGA_1
};
static const uint16_t verticalOffsetStr[9] = {
	VALUE_VERTOFF_0, VALUE_VERTOFF_1, VALUE_VERTOFF_2, VALUE_VERTOFF_3, VALUE_VERTOFF_4,
	VALUE_VERTOFF_5, VALUE_VERTOFF_6, VALUE_VERTOFF_7, VALUE_VERTOFF_8
};
static const uint16_t scanlinesStr[4] = {
	VALUE_SCANLINES_0, VALUE_SCANLINES_1, VALUE_SCANLINES_2, VALUE_SCANLINES_3
};
static const uint16_t spriteLimitStr[2] = {
	VALUE_SPRITELIM_0, VALUE_SPRITELIM_1
};
static const uint16_t audioPresetStr[7] = {
	VALUE_AUDPRESET_0, VALUE_AUDPRESET_1, VALUE_AUDPRESET_2, VALUE_AUDPRESET_3,
	VALUE_AUDPRESET_4, VALUE_AUDPRESET_5, VALUE_AUDPRESET_6
};
static const uint16_t dipCpuStr[2] = {
	VALUE_DIPCPU_0, VALUE_DIPCPU_1
};
static const uint16_t virtualDipVideoStr[4] = {
	VALUE_VDIPVID_0, VALUE_VDIPVID_1, VALUE_VDIPVID_2, VALUE_VDIPVID_3	// for Mapped value
};
static const uint16_t dipVideoStr[4] = {
	VALUE_DIPVID_0, VALUE_DIPVID_1, VALUE_DIPVID_2, VALUE_DIPVID_3		// for Raw value
};
static const uint16_t dipSlot1Str[2] = {
	VALUE_DIPSLOT1_0, VALUE_DIPSLOT1_1
};
static const uint16_t virtualDipSlot2Str[4] = {
	VALUE_VDIPSLOT2_0, VALUE_VDIPSLOT2_1, VALUE_VDIPSLOT2_2, VALUE_VDIPSLOT2_3	// for Mapped value
};
static const uint16_t dipSlot2Str[4] = {
	VALUE_DIPSLOT2_0, VALUE_DIPSLOT2_1, VALUE_DIPSLOT2_2, VALUE_DIPSLOT2_3		// for Raw value
};
static const uint16_t dipMapperStr[2] = {
	VALUE_DIPMAPPER_0, VALUE_DIPMAPPER_1
};


// ========================================================
// ANCHOR: Elements for System Info panel

static const Element_t elemSystem[] = {
	// 0
	{
		LABEL, 
		3,5, LABEL_SYS_FREQUENCIES
	},
	// 1
	{
		CUSTOM_CPUCLOCK_VALUE,
		3,7, LABEL_SYS_CPU_CLOCK,
		5, 1, 7, 7,
		&customCpuClockValue, 0b00001111, 0,10, cpuClockStr, 18,
		CMDTYPE_NONE,
		{ 0x00 }, 
		false,
		{ DESC_CPU_CLOCK_L1, DESC_CPU_CLOCK_L2, ARRAYEND },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,8, LABEL_SYS_CPU_MODE,
		-1, 1, 7, 7,
		&customCpuModeValue, 0b00000011, 0,2, cpuModeStr, 19,
		CMDTYPE_CUSTOM_CPUMODE, 
		{ OCM_SMART_CPU358MHz, OCM_SMART_TurboPana, OCM_SMART_NullCommand }, 
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE | ATR_SENDSMARTLAST,
		{ DESC_CPU_MODE_L1, DESC_CPU_MODE_L2, DESC_CPU_MODE_L3 },
		IOREV_ALL, M_ALL
	},
	// 3	(referenced by CUSTOM_SPEED_IDX)
	{
		SLIDER,
		3,9, LABEL_SYS_CUSTOM_SPEED,
		-1, 1, 6, 6,
		&(sysInfo0.raw), 0b00000111, 1,7, customSpeedStr, 14,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CPU410MHz, OCM_SMART_CPU410MHz, OCM_SMART_CPU448MHz, OCM_SMART_CPU490MHz, 
		  OCM_SMART_CPU539MHz, OCM_SMART_CPU610MHz, OCM_SMART_CPU696MHz, OCM_SMART_CPU806MHz }, 
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE | ATR_SENDSMARTLAST,
		{ DESC_CUSTOM_SPEED_L1, DESC_CUSTOM_SPEED_L2, DESC_CUSTOM_SPEED_L3 },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,10, LABEL_SYS_EXT_BUS_CLOCK,
		-1, 1, 5, 5,
		&(sysInfo2.raw), 0b00000010, 0,1, extBusStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ExtBusCPU, OCM_SMART_ExtBus358 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_EXT_BUS_CLOCK_L1, DESC_EXT_BUS_CLOCK_L2, DESC_EXT_BUS_CLOCK_L3 },
		IOREV_3, M_ALL
	},
	// 5
	{
		SLIDER,
		3,12, LABEL_SYS_TPANA_REDIR,
		-1, 1, 6, 6,
		&(sysInfo0.raw), 0b00010000, 0,1, onOffStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_TPanaRedOFF, OCM_SMART_TPanaRedON },
		ATR_SAVEINPROFILE,
		{ DESC_TPANA_REDIR_L1, DESC_TPANA_REDIR_L2, DESC_TPANA_REDIR_L3 },
		IOREV_1, M_ALL
	},
	// 6
	{
		SLIDER,
		3,13, LABEL_SYS_TURBO_MEGASD,
		-1, -5, 5, 5,
		&(sysInfo0.raw), 0b00001000, 0,1, onOffStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_TMegaSDOFF, OCM_SMART_TMegaSDON },
		ATR_SAVEINPROFILE,
		{ DESC_TURBO_MEGASD_L1, DESC_TURBO_MEGASD_L2, DESC_TURBO_MEGASD_L3 },
		IOREV_1, M_ALL
	},
	// 7
	{
		LABEL, 
		42,5, LABEL_SYS_KEYBOARD_SECTION
	},
	// 8
	{
		VALUE,
		42,7, LABEL_SYS_DEFAULT_KEYBOARD,
		3, 1, -7, -7,
		&(pldVers1.raw), 0b10000000, 0,1, keyboardStr, 27,
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ DESC_DEFAULT_KEYBOARD_L1, DESC_DEFAULT_KEYBOARD_L2, DESC_DEFAULT_KEYBOARD_L3 },
		IOREV_1, M_ALL
	},
	// 9
	{
		SLIDER,
		42,8, LABEL_SYS_CURRENT_KEYBOARD,
		-1, 2, -7, -7,
		&(sysInfo1.raw), 0b00000010, 0,1, keyboardStr, 20,
		CMDTYPE_STANDARD,
		{ OCM_SMART_KBLayoutJP, OCM_SMART_KBLayoutNJP },
		ATR_SAVEINPROFILE,
		{ DESC_CURRENT_KEYBOARD_L1, DESC_CURRENT_KEYBOARD_L2, DESC_CURRENT_KEYBOARD_L3 },
		IOREV_1, M_ALL
	},
	// 10
	{
		LABEL, 
		42,11, LABEL_SYS_SYSTEM_SECTION
	},
	// 11
	{
		BUTTON,
		42,13, LABEL_SYS_RESET_DEFAULTS,
		-2, -3, -5, -5,
		NULL, 0, 0,0, NULL, 0,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ResetDefaults },
		ATR_FORCEPANELRELOAD | ATR_AREYOUSURE,
		{ DESC_RESTORE_DEFAULTS_L1, DESC_RESTORE_DEFAULTS_L2, DESC_RESTORE_DEFAULTS_L3 },
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
		3,6, LABEL_VID_VIDEO_MODE,
		6, 1, 0, 0,
		&customVideoModeValue, 0b00000011, 0,2, videoModeStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_ForcePAL, OCM_SMART_VideoAuto, OCM_SMART_ForceNTSC },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_VIDEO_MODE_L1, DESC_VIDEO_MODE_L2, DESC_VIDEO_MODE_L3 },
		IOREV_4, M_ALL
	},
	// 1
	{
		SLIDER,
		3,8, LABEL_VID_LEGACY_OUTPUT,
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00100000, 0,1, legacyVgaStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_LegacyVGA, OCM_SMART_LegacyVGAplus },
		false,
		{ DESC_LEGACY_OUTPUT_L1, ARRAYEND },
		IOREV_7, M_ALL
	},
	// 2
	{
		SLIDER,
		3,9, LABEL_VID_SCANLINES,
		-1, 1, 0, 0,
		&(sysInfo4_0.raw), 0b00000011, 0,3, scanlinesStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_Scanlines00, OCM_SMART_Scanlines25, OCM_SMART_Scanlines50, OCM_SMART_Scanlines75 },
		ATR_SAVEINPROFILE,
		{ DESC_SCANLINES_L1, ARRAYEND },
		IOREV_10, M_SECOND_GEN
	},
	// 3
	{
		SLIDER,
		3,11, LABEL_VID_VERTICAL_OFFSET,
		-1, 1, 0, 0,
		&(customVerticalOffsetValue), 0b00001111, 0,8, verticalOffsetStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VertOffset16, OCM_SMART_VertOffset17, OCM_SMART_VertOffset18,
		  OCM_SMART_VertOffset19, OCM_SMART_VertOffset20, OCM_SMART_VertOffset21,
		  OCM_SMART_VertOffset22, OCM_SMART_VertOffset23, OCM_SMART_VertOffset24 },
		ATR_SAVEINPROFILE,
		{ DESC_VERTICAL_OFFSET_L1, DESC_VERTICAL_OFFSET_L2, DESC_VERTICAL_OFFSET_L3 },
		IOREV_8, M_ALL
	},
	// 4
	{
		SLIDER,
		3,13, LABEL_VID_VDP_SPEED,
		-1, 1, 0, 0,
		&(sysInfo0.raw), 0b00100000, 0,1, vdpSpeedStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_VDPNormal, OCM_SMART_VDPFast },
		ATR_SAVEINPROFILE,
		{ DESC_VDP_SPEED_L1, DESC_VDP_SPEED_L2, DESC_VDP_SPEED_L3 },
		IOREV_2, M_ALL
	},
	// 5
	{
		SLIDER,
		3,14, LABEL_VID_CENTER_YJK,
		-1, 1, 0, 0,
		&(sysInfo3.raw), 0b00010000, 0,1, onOffStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CenterYJKOFF, OCM_SMART_CenterYJKON },
		ATR_SAVEINPROFILE,
		{ DESC_CENTER_YJK_MODES_L1, DESC_CENTER_YJK_MODES_L2, DESC_CENTER_YJK_MODES_L3 },
		IOREV_7, M_ALL
	},
	// 6
	{
		SLIDER,
		3,15, LABEL_VID_SPRITE_LIMIT,
		-1, -6, 0, 0,
		&(sysInfo4_2.raw), 0b00000001, 0,1, spriteLimitStr, 25,
		CMDTYPE_STANDARD,
		{ OCM_SMART_SpriteLimit48, OCM_SMART_SpriteLimit88 },
		ATR_SAVEINPROFILE,
		{ DESC_SPRITE_LIMIT_L1, DESC_SPRITE_LIMIT_L2, DESC_SPRITE_LIMIT_L3 },
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
		3,6, LABEL_AUD_PRESETS,
		7, 1, 8, 8,
		&customAudioPresetValue, 0b00000111, 0,6, audioPresetStr, 14,
		CMDTYPE_STANDARD,
		{ OCM_SMART_NullCommand, OCM_SMART_AudioPreset1, OCM_SMART_AudioPreset2, OCM_SMART_AudioPreset3,
		  OCM_SMART_AudioPreset4, OCM_SMART_AudioPreset5, OCM_SMART_AudioPreset6 },
		ATR_FORCEPANELRELOAD,
		{ DESC_AUDIO_PRESETS_L1, DESC_AUDIO_PRESETS_L2, DESC_AUDIO_PRESETS_L3 },
		IOREV_7, M_ALL
	},
	// 1
	{
		CUSTOM_VOLUME_SLIDER,
		3,8, LABEL_AUD_MASTER_VOL,
		-1, 1, 8, 8,
		&(audioVols0.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_MasterVol0, OCM_SMART_MasterVol1, OCM_SMART_MasterVol2, OCM_SMART_MasterVol3,
		  OCM_SMART_MasterVol4, OCM_SMART_MasterVol5, OCM_SMART_MasterVol6, OCM_SMART_MasterVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_MASTER_VOLUME_L1, DESC_MASTER_VOLUME_L2, ARRAYEND },
		IOREV_4, M_ALL
	},
	// 2
	{
		CUSTOM_VOLUME_SLIDER,
		3,9, LABEL_AUD_PSG_VOL,
		-1, 1, 7, 7,
		&(audioVols0.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PSGVol0, OCM_SMART_PSGVol1, OCM_SMART_PSGVol2, OCM_SMART_PSGVol3,
		  OCM_SMART_PSGVol4, OCM_SMART_PSGVol5, OCM_SMART_PSGVol6, OCM_SMART_PSGVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_PSG_VOLUME_L1, DESC_PSG_VOLUME_L2, ARRAYEND },
		IOREV_4, M_ALL
	},
	// 3
	{
		CUSTOM_VOLUME_SLIDER,
		3,10, LABEL_AUD_SCC_VOL,
		-1, 1, 6, 6,
		&(audioVols1.raw), 0b01110000, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_SCCIVol0, OCM_SMART_SCCIVol1, OCM_SMART_SCCIVol2, OCM_SMART_SCCIVol3,
		  OCM_SMART_SCCIVol4, OCM_SMART_SCCIVol5, OCM_SMART_SCCIVol6, OCM_SMART_SCCIVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_SCC_VOLUME_L1, DESC_SCC_VOLUME_L2, ARRAYEND },
		IOREV_4, M_ALL
	},
	// 4
	{
		CUSTOM_VOLUME_SLIDER,
		3,11, LABEL_AUD_OPLL_VOL,
		-1, 1, 5, 5,
		&(audioVols1.raw), 0b00000111, 0,7, numbersStr, 18,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPLLVol0, OCM_SMART_OPLLVol1, OCM_SMART_OPLLVol2, OCM_SMART_OPLLVol3, 
		  OCM_SMART_OPLLVol4, OCM_SMART_OPLLVol5, OCM_SMART_OPLLVol6, OCM_SMART_OPLLVol7 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_OPLL_VOLUME_L1, DESC_OPLL_VOLUME_L2, ARRAYEND },
		IOREV_4, M_ALL
	},
	// 5
	{
		SLIDER,
		3,13, LABEL_AUD_PSG2,
		-1, 1, 4, 4,
		&(sysInfo4_0.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_IntPSG2OFF, OCM_SMART_IntPSG2ON },
		ATR_SAVEINPROFILE,
		{ DESC_PSG2_L1, DESC_PSG2_L2, ARRAYEND },
		IOREV_11, M_SECOND_GEN
	},
	// 6
	{
		SLIDER,
		3,15, LABEL_AUD_OPL3,
		-1, 1, 3, 3,
		&(sysInfo1.raw), 0b00000100, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_OPL3OFF, OCM_SMART_OPL3ON },
		ATR_SAVEINPROFILE,
		{ DESC_OPL3_L1, DESC_OPL3_L2, ARRAYEND },
		IOREV_10, M_SECOND_GEN
	},
	// 7
	{
		SLIDER,
		3,17, LABEL_AUD_CMT_IF,
		-1, -7, 2, 2,
		&(sysInfo1.raw), 0b00000100, 0,1, cmtOnOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_CMTOFF, OCM_SMART_CMTON },
		ATR_SAVEINPROFILE|ATR_USELASTSTRFORNA,
		{ DESC_CMT_IF_L1, DESC_CMT_IF_L2, DESC_CMT_IF_L3 },
		IOREV_ALL, M_FIRST_GEN
	},
	// 8
	{
		SLIDER,
		40,6, LABEL_AUD_PSEUDO_STEREO,
		1, 1, -8, -8,
		&(sysInfo2.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_PseudSterOFF, OCM_SMART_PseudSterON },
		ATR_SAVEINPROFILE,
		{ DESC_PSEUDO_STEREO_L1, DESC_PSEUDO_STEREO_L2, ARRAYEND },
		IOREV_3, M_ALL
	},
	// 9
	{
		SLIDER,
		40,8, LABEL_AUD_RIGHT_INVERSE,
		-1, -1, -8, -8,
		&(sysInfo3.raw), 0b00000001, 0,1, onOffStr, 24,
		CMDTYPE_STANDARD,
		{ OCM_SMART_RightInvAud0, OCM_SMART_RightInvAud1 },
		ATR_SAVEINPROFILE,
		{ DESC_RIGHT_INVERSE_AUDIO_L1, DESC_RIGHT_INVERSE_AUDIO_L2, DESC_RIGHT_INVERSE_AUDIO_L3 },
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
		3,5, LABEL_DIP_VIRTUAL_SECTION
	},
	// 1
	{
		SLIDER,
		3,7, LABEL_DIP_V_CPU_CLOCK,
		5, 1, 7, 7,
		&(virtualDIPs.raw), 0b00000001, 0,1, dipCpuStr, 20, 
		CMDTYPE_CUSTOM_CPUMODE,
		{ OCM_SMART_CPU358MHz, OCM_SMART_NullCommand },
		ATR_FORCEPANELRELOAD,
		{ DESC_V_CPU_CLOCK_L1, DESC_V_CPU_CLOCK_L2, DESC_V_CPU_CLOCK_L3 },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,9, LABEL_DIP_V_VIDEO_OUTPUT,
		-1, 1, 7, 7,
		&customVideoOutputValue, 0b00000011, 0,3, virtualDipVideoStr, 18, 
		CMDTYPE_STANDARD,
		{ OCM_SMART_Disp15KhSvid, OCM_SMART_Disp15KhRGB, OCM_SMART_Disp31KhVGA, OCM_SMART_Disp31KhVGAp },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_V_VIDEO_OUTPUT_L1, DESC_V_VIDEO_OUTPUT_L2, DESC_V_VIDEO_OUTPUT_L3},
		IOREV_ALL, M_ALL
	},
	// 3
	{
		SLIDER,
		3,11, LABEL_DIP_V_SLOT1,
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000001, 0,1, dipSlot1Str, 20, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
		  OCM_SMART_S1extS2a16, OCM_SMART_S1sccS2a16 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_V_CARTRIDGE_SLOT1_L1, DESC_V_CARTRIDGE_SLOT1_L2, DESC_V_CARTRIDGE_SLOT1_L3 },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,13, LABEL_DIP_V_SLOT2,
		-1, 1, 7, 7,
		&customSlots12Value, 0b00000110, 0,3, virtualDipSlot2Str, 18, 
		CMDTYPE_CUSTOM_SLOTS12,
		{ OCM_SMART_S1extS2ext, OCM_SMART_S1sccS2ext,
		  OCM_SMART_S1extS2a8,  OCM_SMART_S1sccS2a8,
		  OCM_SMART_S1extS2scc, OCM_SMART_S1sccS2scc,
		  OCM_SMART_S1extS2a16, OCM_SMART_S1sccS2a16 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_V_CARTRIDGE_SLOT2_L1, DESC_V_CARTRIDGE_SLOT2_L2, DESC_V_CARTRIDGE_SLOT2_L3 },
		IOREV_ALL, M_ALL
	},
	// 5
	{
		VALUE,
		3,15, LABEL_DIP_V_MAPPER,
		-1, 1, 7, 7,
		&(virtualDIPs.raw), 0b01000000, 0,1, dipMapperStr, 27,
		CMDTYPE_NONE,
/*REV*/	/*{ OCM_SMART_Mapper4MbOFF, OCM_SMART_Mapper4MbON },
		ATR_FORCEPANELRELOAD | ATR_NEEDRESETTOAPPLY,*/ {0x00}, false,
		{ DESC_V_RAM_MAPPER_L1, DESC_V_RAM_MAPPER_L2, DESC_V_RAM_MAPPER_L3 },
		IOREV_ALL, M_ALL
	},
	// 6
	{
		VALUE,
		3,17, LABEL_DIP_V_MEGASD,
		-1, -5, 7, 7,
		&(virtualDIPs.raw), 0b10000000, 0,1, onOffStr, 27,
		CMDTYPE_NONE,
/*REV*/	/*{ OCM_SMART_MegaSDOFF, OCM_SMART_MegaSDON },
		ATR_FORCEPANELRELOAD | ATR_NEEDRESETTOAPPLY,*/ {0x00}, false,
		{ DESC_V_INTERNAL_MEGASD_L1, DESC_V_INTERNAL_MEGASD_L2, DESC_V_INTERNAL_MEGASD_L3 },
		IOREV_ALL, M_ALL
	},
	// 7
	{
		LABEL, 
		42,5, LABEL_DIP_HARDWARE_SECTION
	},
	// 8
	{
		VALUE,
		42,7, LABEL_DIP_H_CPU_CLOCK,
		5, 1, -7, -7,
		&(sysInfo5.raw), 0b00000001, 0,1, dipCpuStr, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ DESC_H_CPU_CLOCK_L1, DESC_H_CPU_CLOCK_L2, DESC_H_CPU_CLOCK_L3 },
		IOREV_5, M_ALL
	},
	// 9
	{
		VALUE,
		42,9, LABEL_DIP_H_VIDEO_OUTPUT,
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00000110, 0,3, dipVideoStr, 20, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ DESC_H_VIDEO_OUTPUT_L1, DESC_H_VIDEO_OUTPUT_L2, DESC_H_VIDEO_OUTPUT_L3 },
		IOREV_5, M_ALL
	},
	// 10
	{
		VALUE,
		42,11, LABEL_DIP_H_SLOT1,
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00001000, 0,1, dipSlot1Str, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ DESC_H_CARTRIDGE_SLOT1_L1, DESC_H_CARTRIDGE_SLOT1_L2, DESC_H_CARTRIDGE_SLOT1_L3 },
		IOREV_5, M_ALL
	},
	// 11
	{
		VALUE,
		42,13, LABEL_DIP_H_SLOT2,
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b00110000, 0,3, dipSlot2Str, 20, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ DESC_H_CARTRIDGE_SLOT2_L1, DESC_H_CARTRIDGE_SLOT2_L2, DESC_H_CARTRIDGE_SLOT2_L3 },
		IOREV_5, M_ALL
	},
	// 12
	{
		VALUE,
		42,15, LABEL_DIP_H_MAPPER,
		-1, 1, -7, -7,
		&(sysInfo5.raw), 0b01000000, 0,1, dipMapperStr, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ DESC_H_RAM_MAPPER_L1, DESC_H_RAM_MAPPER_L2, DESC_H_RAM_MAPPER_L3 },
		IOREV_5, M_ALL
	},
	// 13
	{
		VALUE,
		42,17, LABEL_DIP_H_MEGASD,
		-1, -5, -7, -7,
		&(sysInfo5.raw), 0b10000000, 0,1, onOffStr, 22, 
		CMDTYPE_NONE,
		{ 0x00 },
		false,
		{ DESC_H_INTERNAL_MEGASD_L1, DESC_H_INTERNAL_MEGASD_L2, DESC_H_INTERNAL_MEGASD_L3 },
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
		3,6, LABEL_LCK_LOCK_ALL,
		4, 1, 0, 0,
		&customLockAllToggles, 0b00000001, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockAll, OCM_SMART_LockAll },
		ATR_FORCEPANELRELOAD,
		{ DESC_LOCK_ALL_TOGGLES_L1, DESC_LOCK_ALL_TOGGLES_L2, DESC_LOCK_ALL_TOGGLES_L3 },
		IOREV_ALL, M_ALL
	},
	// 1
	{
		SLIDER,
		3,9, LABEL_LCK_LOCK_CPU,
		-1, 1, 4, 4,
		&(lockToggles.raw), 0b00000001, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockTurbo, OCM_SMART_LockTurbo },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_CPU_MODE_L1, DESC_LOCK_CPU_MODE_L2, ARRAYEND },
		IOREV_ALL, M_ALL
	},
	// 2
	{
		SLIDER,
		3,11, LABEL_LCK_LOCK_VIDEO,
		-1, 1, 4, 4,
		&(lockToggles.raw), 0b00000010, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockDisplay, OCM_SMART_LockDisplay },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_VIDEO_OUTPUT_L1, DESC_LOCK_VIDEO_OUTPUT_L2, DESC_LOCK_VIDEO_OUTPUT_L3 },
		IOREV_ALL, M_ALL
	},
	// 3
	{
		SLIDER,
		3,13, LABEL_LCK_LOCK_AUDIO,
		-1, 1, 4, 4,
		&(lockToggles.raw), 0b00000100, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockAudio, OCM_SMART_LockAudio },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_AUDIO_MIXER_L1, DESC_LOCK_AUDIO_MIXER_L2, DESC_LOCK_AUDIO_MIXER_L3 },
		IOREV_ALL, M_ALL
	},
	// 4
	{
		SLIDER,
		3,15, LABEL_LCK_LOCK_RESET,
		-1, -4, 4, 4,
		&(lockToggles.raw), 0b00100000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockHardRst, OCM_SMART_LockHardRst },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_RESET_KEY_L1, DESC_LOCK_RESET_KEY_L2, DESC_LOCK_RESET_KEY_L3 },
		IOREV_ALL, M_ALL
	},
	// 5
	{
		SLIDER,
		42,9, LABEL_LCK_LOCK_SLOT1,
		3, 1, -4, -4,
		&(lockToggles.raw), 0b00001000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockSlot1, OCM_SMART_LockSlot1 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_SLOT1_L1, DESC_LOCK_SLOT1_L2, ARRAYEND },
		IOREV_ALL, M_ALL
	},
	// 6
	{
		SLIDER,
		42,11, LABEL_LCK_LOCK_SLOT2,
		-1, 1, -4, -4,
		&(lockToggles.raw), 0b00010000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockSlot2, OCM_SMART_LockSlot2 },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_SLOT2_L1, DESC_LOCK_SLOT2_L2, ARRAYEND },
		IOREV_ALL, M_ALL
	},
	// 7
	{
		SLIDER,
		42,13, LABEL_LCK_LOCK_MAPPER,
		-1, 1, -4, -4,
		&(lockToggles.raw), 0b01000000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockMapper, OCM_SMART_LockMapper },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_INTERNAL_MAPPER_L1, DESC_LOCK_INTERNAL_MAPPER_L2, ARRAYEND },
		IOREV_ALL, M_ALL
	},
	// 8
	{
		SLIDER,
		42,15, LABEL_LCK_LOCK_MEGASD,
		-1, -3, -4, -4,
		&(lockToggles.raw), 0b10000000, 0,1, onOffStr, 23,
		CMDTYPE_STANDARD,
		{ OCM_SMART_UnlockMegaSD, OCM_SMART_LockMegaSD },
		ATR_FORCEPANELRELOAD | ATR_SAVEINPROFILE,
		{ DESC_LOCK_INTERNAL_MEGASD_L1, DESC_LOCK_INTERNAL_MEGASD_L2, ARRAYEND },
		IOREV_ALL, M_ALL
	},
	// END
	{ END }
};

// ========================================================
// ANCHOR: Elements for Help panel

static const Element_t elemHelp[] = {
	{ LABEL, 3,5, LABEL_HLP_TITLE,
		0, 0, 0, 0, NULL, 0, 0,0, NULL, 0, CMDTYPE_NONE,{ 0x00 }, false, 
		{ DESC_HELP_L1, DESC_HELP_L2, DESC_HELP_L3 }
	},
	{ LABEL, 3,7,  LABEL_HLP_LINE1 },
	{ LABEL, 3,8,  LABEL_HLP_LINE2 },

	{ LABEL, 3,10, LABEL_HLP_LINE3 },
	{ LABEL, 3,11, LABEL_HLP_LINE4 },
	{ LABEL, 3,12, LABEL_HLP_LINE5 },
	{ LABEL, 3,13, LABEL_HLP_LINE6 },

	{ LABEL, 3,15, LABEL_HLP_LINE7 },	

	{ LABEL, 3,17, LABEL_HLP_LINE8 },
	{ LABEL, 3,18, LABEL_HLP_LINE9 },
	// END
	{ END }
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
	{ MENU_SYSTEM,		2,3, 	11,	elemSystem },
	{ MENU_VIDEO,		12,3, 	10,	elemVideo },
	{ MENU_AUDIO,		21,3,	10,	elemAudio },
	{ MENU_DIPS,		30,3,	11,	elemDIPs },
	{ MENU_LOCKS,		40,3,	10,	elemLocks },
	{ MENU_ABOUT,		53,3,	9,	elemHelp },
	{ MENU_PROFILES,	61,3,	12,	NULL },
	{ MENU_EXIT,		72,3,	8,	NULL },
	{ ARRAYEND }
};


// ========================================================
// Dialogs

const uint16_t dlg_exitStr[] = {
	DLG_EXIT_TITLE, DLG_EXIT_TEXT1, DLG_EXIT_TEXT2, ARRAYEND
};
const Dialog_t dlg_exit = {
	0,0,
	dlg_exitStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_NO,		//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_resetStr[] = {
	DLG_RESET_TITLE, DLG_RESET_TEXT1, DLG_RESET_TEXT2, DLG_RESET_TEXT3, ARRAYEND
};
const Dialog_t dlg_reset = {
	0,0,
	dlg_resetStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_YES,	//cancelButton
	DLG_DEFAULT
};

const uint16_t dlg_confirmStr[] = {
	DLG_CONFIRM_TITLE, DLG_CONFIRM_TEXT1, DLG_CONFIRM_TEXT2, ARRAYEND
};
const Dialog_t dlg_confirm = {
	0,0,
	dlg_confirmStr,
	dlg_yesNoBtn,
	BTN_YES,	//defaultButton
	BTN_NO,		//cancelButton
	DLG_DEFAULT
};
