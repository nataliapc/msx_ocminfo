/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
// Expanded Switched I/O ports
// http://map.grauw.nl/resources/msx_io_ports.php#switch_io
// https://github.com/gnogni/ocm-pld-dev/tree/master/docs <-- switched io ports revision 12.pdf
// https://github.com/cayce-msx/msxpp-quick-ref/wiki
#pragma once
#include <stdint.h>
#include <stdbool.h>


// ========================================================
//  Ports
#define EXTIO_DEVICEID_PORT		0x40
#define OCM_SMARTCMD_PORT		0x41
#define OCM_VIRTDIPS_PORT		0x42
#define OCM_LOCKTOGG_PORT		0x43
#define OCM_LEDLIGHT_PORT		0x44
#define OCM_AUDVOLS0_PORT		0x45
#define OCM_AUDVOLS1_PORT		0x46
#define OCM_SYSINFO0_PORT		0x47
#define OCM_SYSINFO1_PORT		0x48
#define OCM_SYSINFO2_PORT		0x49
#define OCM_SYSINFO3_PORT		0x4a
#define OCM_SYSINFO4_PORT		0x4b
#define OCM_SYSINFO5_PORT		0x4c
#define OCM_PLDVERS0_PORT		0x4e
#define OCM_PLDVERS1_PORT		0x4f

__sfr __at EXTIO_DEVICEID_PORT expIO_deviceId;	// Expanded IO Devices port     (read_n / write)
__sfr __at OCM_SMARTCMD_PORT ocm_smartCmd;		// [OCM] Smart Command ID       (read_n / write)
__sfr __at OCM_VIRTDIPS_PORT ocm_virtualDIPs;	// [OCM] Virtual DIPs           (read / write_n)
__sfr __at OCM_LOCKTOGG_PORT ocm_lockToggles;	// [OCM] Led Lights             (read / write_n)
__sfr __at OCM_LEDLIGHT_PORT ocm_ledLights;		// [OCM] Led Lights             (read / write_n)
__sfr __at OCM_AUDVOLS0_PORT ocm_volumes0;		// [OCM] Audio Volumes #0       (read / write_n)
__sfr __at OCM_AUDVOLS1_PORT ocm_volumes1;		// [OCM] Audio Volumes #1       (read / write_n)
__sfr __at OCM_SYSINFO0_PORT ocm_sysInfo0;		// [OCM] Get System Info #0     (read only)
__sfr __at OCM_SYSINFO1_PORT ocm_sysInfo1;		// [OCM] Get System Info #1     (read only)
__sfr __at OCM_SYSINFO2_PORT ocm_sysInfo2;		// [OCM] Get System Info #2     (read only)
__sfr __at OCM_SYSINFO3_PORT ocm_sysInfo3;		// [OCM] Get System Info #3     (read only)
__sfr __at OCM_SYSINFO4_PORT ocm_sysInfo4;		// [OCM] Get System Info #4     (read only)
__sfr __at OCM_SYSINFO5_PORT ocm_sysInfo5;		// [OCM] Get System Info #5     (read only)
__sfr __at OCM_PLDVERS0_PORT ocm_pldVersion0;	// [OCM] Get OCM-PLD Version #0 (read only)
__sfr __at OCM_PLDVERS1_PORT ocm_pldVersion1;	// [OCM] Get OCM-PLD Version #1 (read only)


// ========================================================
//  Data structs

typedef union {							// [OCM] Virtual DIPs           (read / write_n)
	struct {
		unsigned cpuClock: 1;			// bit 0    CPU Clock                      | Virtual DIP-SW1
		unsigned videoOutput_raw: 2;	// bit 1    Video Output (MSB)             | Virtual DIP-SW2
										// bit 2    Video Output (LSB)             | Virtual DIP-SW3
		unsigned cartSlot1: 1;			// bit 3    Cartridge slot 1               | Virtual DIP-SW4
		unsigned cartSlot2_raw: 2;		// bit 4    Cartridge slot 2 (MSB)         | Virtual DIP-SW5
										// bit 5    Cartridge slot 2 (LSB)         | Virtual DIP-SW6
		unsigned internalMapper: 1;		// bit 6    Internal Mapper                | Virtual DIP-SW7
		unsigned internalMegaSD: 1;		// bit 7    Internal MegaSD                | Virtual DIP-SW8
	};
	uint8_t raw;
} OCM_P42_VirtualDIP_t;

typedef union {							// [OCM] Lock Mask Toggles      (read / write_n)
	struct {
		unsigned lockCpuClock: 1;		// bit 0   Lock CPU Clock                  | RW Status
		unsigned lockVideoOutput: 1;	// bit 1   Lock Video Output               | RW Status
		unsigned lockAudioMixer: 1;		// bit 2   Lock Audio Mixer & SCRLK        | RW Status
		unsigned lockCartSlot1: 1;		// bit 3   Lock Cartridge Slot-1           | RW Status
		unsigned lockCartSlot2: 1;		// bit 4   Lock Cartridge Slot-2           | RW Status
		unsigned lockHardResetKey: 1;	// bit 5   Lock Hard Reset Key             | RW Status
		unsigned lockIntMapper: 1;		// bit 6   Lock Internal Mapper            | RW Status
		unsigned lockIntMegaSD: 1;		// bit 7   Lock Internal MegaSD            | RW Status
	};
	uint8_t raw;
} OCM_P43_LockToggles_t;

typedef union {							// [OCM] Led Lights             (read / write_n)
	struct {
		unsigned led1: 1;				// bit 0   Led 1                           | RW Status
		unsigned led2: 1;				// bit 1   Led 2                           | RW Status
		unsigned led3: 1;				// bit 2   Led 3                           | RW Status
		unsigned led4: 1;				// bit 3   Led 4                           | RW Status
		unsigned led5: 1;				// bit 4   Led 5                           | RW Status
		unsigned led6: 1;				// bit 5   Led 6                           | RW Status
		unsigned led7: 1;				// bit 6   Led 7                           | RW Status
		unsigned led8: 1;				// bit 7   Led 8                           | RW Status
	};
	uint8_t raw;
} OCM_P44_LedLights_t;

typedef union {							// [OCM] Audio Volumes #0       (read / write_n)
	struct {
		unsigned psgVolumeRW: 3;		// bit 0-2 PSG Volume level                | RW (range 0-7)
		unsigned psgMuteRO: 1;			// bit 3   PSG Mute                        | RO Status
		unsigned masterVolumeRW: 3;		// bit 4-6 Master Volume level             | RW (range 0-7)
		unsigned masterMuteRO: 1;		// bit 7   Master Mute                     | RO Status
	};
	uint8_t raw;
} OCM_P45_AudioVol0_t;

typedef union {							// [OCM] Audio Volumes #1       (read / write_n)
	struct {
		unsigned opllVolumeRW: 3;		// bit 0-2 OPLL Volume level               | RW (range 0-7)
		unsigned opllMuteRO: 1;			// bit 3   OPLL Mute                       | RO Status
		unsigned scciVolumeRW: 3;		// bit 4-6 SCC-I Volume level              | RW (range 0-7)
		unsigned scciMuteRO: 1;			// bit 7   SCC-I Mute                      | RO Status
	};
	uint8_t raw;
} OCM_P46_AudioVol1_t;

typedef union {							// [OCM] Get System Info #0     (read only)
	struct {
		unsigned cpuCustomSpeed: 3;		// bit 0-2 CPU Custom Speed (bit 0 LSB)    |
										//            0=3.58MHz, 1=4.10MHz, 2=4.48MHz, 3=4.90MHz, 
										//            4=5.39MHz, 5=6.10MHz, 6=6.96MHz, 7=8.06MHz
		unsigned turboMegaSD: 1;		// bit 3   Turbo MegaSD (tMSD)             | Status
		unsigned turboPanaRedirec: 1;	// bit 4   Turbo Pana Redirection (tPR)    | Status
		unsigned vdpSpeed: 1;			// bit 5   VDP Speed Mode                  | 0=Normal, 1=Fast
		unsigned mapperSizeReq: 1;		// bit 6   Mapper Size Req                 | 0=2048KB, 1=4096KB
		unsigned megaSDmodeReq: 1;		// bit 7   MegaSD Mode Req                 | Status
	};
	uint8_t raw;
} OCM_P47_SysInfo0_t;

typedef union {							// [OCM] Get System Info #1     (read only)
	struct {
		unsigned turboPana: 1;			// bit 0   Turbo Pana                      | Status
		unsigned currKboardLayout: 1;	// bit 1   Current Keyboard Layout         | 0=JP, 1=Non-JP
		unsigned SCRLKtoggle: 1;		// bit 2   SCRLK Toggle                    | Status
		unsigned lightsMode: 1;			// bit 3   Lights Mode                     | 0=Auto, 1=ON
		unsigned redMode: 1;			// bit 4   Red Mode (Led 0)                | Status
		unsigned lastResetFlag: 1;		// bit 5   Last Reset Flag                 | 0=Cold, 1=Warm
		unsigned resetReqFlag: 1;		// bit 6   Reset Required Flag             | Status
		unsigned megaSDblink: 1;		// bit 7   MegaSD Blink                    | Status
	};
	uint8_t raw;
} OCM_P48_SysInfo1_t;

typedef union {							// [OCM] Get System Info #2     (read only)
	struct {
		unsigned pseudoStereo: 1;		// bit 0   Pseudo Stereo                   | Status
		unsigned extClockMode: 1;		// bit 1   External Bus Clock Mode         | 0=Sync to CPU, 1=3.58MHz
		unsigned machineTypeId: 4;		// bit 2-5 Machine Type ID (bit 0 LSB)     |
										//            0=OCM, 1=Zemmix Neo/SX-1, 2=SM-X/MCP2, 3=SX-2,
										//            4=SM-X Mini/SM-X HB, 5=DE0CV, 6=SX-E,
										//            7-14=Free, 15=Unknown
		unsigned videoType: 1;			// bit 6   NTSC/PAL type                   | 0=Forced, 1=Auto
		unsigned videoForcedMode: 1;	// bit 7   Forced Video Mode               | 0=60Hz (NTSC), 1=50Hz (PAL)
	};
	uint8_t raw;
} OCM_P49_SysInfo2_t;

typedef union {							// [OCM] Get System Info #3     (read only)
	struct {
		unsigned rightInvAudio: 1;		// bit 0   Right Inverse Audio             | Status
		unsigned pixelRatioDisplay: 3;	// bit 1   Pixel Ratio 1:1 Display         | default:0|range:0-7|60Hz only
		unsigned centerYJK_R25: 1;		// bit 4   Centering YJK Modes/R25 Mask    | Status
		unsigned legacyOutput: 1;		// bit 5   Assignment of Legacy Output     | 0=To VGA, 1=To VGA+
		unsigned intSlot1Linear: 1;		// bit 6   Internal Slot-1 Linear          | Status
		unsigned intSlot2Linear: 1;		// bit 7   Internal Slot-2 Linear          | Status
	};
	uint8_t raw;
} OCM_P4A_SysInfo3_t;

typedef union {							// [OCM] Get System Info #4     (read only) [if port 0x44 == 0]
	struct {
		unsigned scanlinesLvl: 2;		// bit 0-1 VGA Scanlines Level (SM-X/SX-2) | 0=0%, 1=25%, 2=50%, 3=75%
		unsigned intPSG2: 1;			// bit 2   Internal PSG2       (SM-X/SX-2) | Status
		unsigned sdramSize: 2;			// bit 3-4 SDRAM Size                      | 
										//            0=8MB, 1=16MB, 2=32MB, 3=Use sdramSizeAux
		unsigned ocmBiosReloadReq: 1;	// bit 5   OCM-BIOS Reloading Req          | Status
		unsigned extraMapperReq: 1;		// bit 6   Extra-Mapper 4096KB Req         | Status
		unsigned slot0ModeReq: 1;		// bit 7   Slot-0 Mode Req                 | 0=Primaty, 1=Expanded
	};
	struct {
		uint8_t raw;
		uint8_t errorFlag;
	};
} OCM_P4B_SysInfo4_0_t;

typedef union {							// [OCM] Get System Info #4(b)  (read only) [if port 0x44 == 1]
	struct {
		unsigned extMegaRomReading: 1;	// bit 0   Extended MegaROM Reading        | Status
		unsigned sdramSizeAux: 3;		// bit 1-3 SDRAM Size > 32MB               |
										//            0=64MB, 1=128MB, 2=192MB, 3=256MB, 4=320MB,
										//            5=384MB, 6=448MB, 7=512MB
		unsigned verticalOffset: 4;		// bit 4-7 Vertical screen offset          | [4-12] -> [16-24]
	};
	struct {
		uint8_t raw;
		uint8_t errorFlag;
	};
} OCM_P4B_SysInfo4_1_t;

typedef union {							// [OCM] Get System Info #5     (read only)
	struct {
		unsigned cpuClock: 1;			// bit 0    CPU Clock                      | Hard DIP-SW1
		unsigned videoOutput_1: 1;		// bit 1    Video Output (MSB)             | Hard DIP-SW2
		unsigned videoOutput_0: 1;		// bit 2    Video Output (LSB)             | Hard DIP-SW3
		unsigned cartSlot1: 1;			// bit 3    Cartridge slot 1               | Hard DIP-SW4
		unsigned cartSlot2_1: 1;		// bit 4    Cartridge slot 2 (MSB)         | Hard DIP-SW5
		unsigned cartSlot2_0: 1;		// bit 5    Cartridge slot 2 (LSB)         | Hard DIP-SW6
		unsigned internalMapper: 1;		// bit 6    Internal Mapper                | Hard DIP-SW7
		unsigned internalMegaSD: 1;		// bit 7    Internal MegaSD                | Hard DIP-SW8
	};
	uint8_t raw;
} OCM_P4C_SysInfo5_t;

typedef union							// [OCM] Get OCM-PLD Version #0 (read only)
{
	uint8_t pldVersion;					// bit 0-7  OCM-PLD Main Version           | (range 0.0.z - 25.5.z)
	uint8_t raw;
} OCM_P4E_Version0_t;

typedef union {							// [OCM] Get OCM-PLD Version #1 (read only)
	struct {
		unsigned ioRevision: 5;			// bit 0-4  I/O Revision                   | (range 0 - 31)
		unsigned pldSubversion: 2;		// bit 5-6  OCM-PLD SubVersion             | (range x.y.0 - x.y.3)
		unsigned defKboardLayout: 1;	// bit 7    Default Keyboard Layout        | Status
	};
	uint8_t raw;
} OCM_P4F_Version1_t;

// ========================================================
//  Constants

/*
	#### Device ID register (port 0x40) ####
	The device is determined by the device ID written to port #40.
	When port #40 is read the currently selected device returns the complement
	of the current device ID, if present.
	[1..127]   Manufacturer ID numbers
	[128..254] Device ID numbers
*/
typedef enum {
	// Manufacturer IDs
	DEVID_ASCII = 1,		// 1
	DEVID_CANON,			// 2
	DEVID_CASIO,			// 3
	DEVID_FUJITSU,			// 4
	DEVID_GENERAL,			// 5
	DEVID_HITACHI,			// 6
	DEVID_KYOCERA,			// 7
	DEVID_PANASONIC,		// 8 (Matsushita)
	DEVID_MITSUBISHI,		// 9
	DEVID_NEC,				// 10
	DEVID_YAMAHA,			// 11 (Nippon Gakki)
	DEVID_JVC,				// 12
	DEVID_PHILIPS,			// 13
	DEVID_PIONEER,			// 14
	DEVID_SANYO,			// 15
	DEVID_SHARP,			// 16
	DEVID_SONY,				// 17
	DEVID_SPECTRAVIDEO,		// 18
	DEVID_TOSHIBA,			// 19
	DEVID_MITSUMI,			// 20
	DEVID_TELEMATICA,		// 21
	DEVID_GRADIENTE,		// 22
	DEVID_SHARP_BR,			// 23
	DEVID_GOLDSTAR,			// 24 (LG)
	DEVID_DAEWOO,			// 25
	DEVID_SAMSUNG,			// 26
	// Device IDs
	DEVID_IMG_SCAN = 128,	// (Matsushita)
	DEVID_WORP3 =    165,	// WORP3
	DEVID_DARKY =    170,	// SuperSoniqs
	DEVID_DARKY2 =   171,	// SuperSoniqs second setting
	DEVID_OCMPLD =   212,	// 1chip / Zemmix Neo / OCM Kdl firmware
	DEVID_MPS2 =     254	// MPS2 (ASCII)
} DeviceId_t;

/*
	#### OCM Smart Commands (port 0x41) ####
*/
/*
0x00	// Null Command (reserved)
*/
#define OCM_SMART_TPanaRedOFF	0x01	// Set Turbo Pana Redirection OFF (default)
#define OCM_SMART_TPanaRedON	0x02	// Set Turbo Pana Redirection ON

#define OCM_SMART_CPU358MHz		0x03	// Set Standard Speed 3.58 MHz
#define OCM_SMART_CPU410MHz		0x04	// Set Custom Speed 4.10 MHz
#define OCM_SMART_CPU448MHz		0x05	// Set Custom Speed 4.48 MHz
#define OCM_SMART_CPU490MHz		0x06	// Set Custom Speed 4.90 MHz
#define OCM_SMART_CPU539MHz		0x07	// Set Custom Speed 5.39 MHz
#define OCM_SMART_CPU610MHz		0x08	// Set Custom Speed 6.10 MHz
#define OCM_SMART_CPU696MHz		0x09	// Set Custom Speed 6.96 MHz
#define OCM_SMART_CPU806MHz		0x0a	// Set Custom Speed 8.06 MHz (aka "Turbo 10 MHz")

#define OCM_SMART_TMegaSDOFF	0x0b	// Set Turbo MegaSD OFF
#define OCM_SMART_TMegaSDON		0x0c	// Set Turbo MegaSD ON (default)

#define OCM_SMART_S1extS2ext	0x0d	// Set External Slot-1 + External Slot-2
#define OCM_SMART_S1sccS2ext	0x0e	// Set Internal SCC-I Slot-1 + External Slot-2
#define OCM_SMART_S1extS2scc	0x0f	// Set External Slot-1 + Internal SCC-I Slot-2
#define OCM_SMART_S1sccS2scc	0x10	// Set Internal SCC-I Slot-1 + Internal SCC-I Slot-2
#define OCM_SMART_S1extS2a8		0x11	// Set External Slot-1 + Internal ASCII-8K Slot-2
#define OCM_SMART_S1sccS2a8		0x12	// Set Internal SCC-I Slot-1 + Internal ASCII-8K Slot-2
#define OCM_SMART_S1extS2a16	0x13	// Set External Slot-1 + Internal ASCII-16K Slot-2
#define OCM_SMART_S1sccS2a16	0x14	// Set Internal SCC-I Slot-1 + Internal ASCII-16K Slot-2

#define OCM_SMART_KBLayoutJP	0x15	// Set Japanese Keyboard Layout
#define OCM_SMART_KBLayoutNJP	0x16	// Set Non-Japanese Keyboard Layout

#define OCM_SMART_Disp15KhSvid	0x17	// Set Display Mode 15KHz Composite/S-Video
#define OCM_SMART_Disp15KhRGB	0x18	// Set Display Mode 15KHz RGB w/ Audio Out
#define OCM_SMART_Disp31KhVGA	0x19	// Set Display Mode 31Khz VGA for LED TV or LED Display (also HDMI AV on SM-X)
#define OCM_SMART_Disp31KhVGAp	0x1a	// Set Display Mode 31Khz VGA+ for CRT Monitor (legacy output) (also HDMI AV on SM-X)

#define OCM_SMART_VDPNormal		0x1b	// Set VDP Speed Normal Mode (default)
#define OCM_SMART_VDPFast		0x1c	// Set VDP Speed Fast Mode (V9958 only)

#define OCM_SMART_MegaSDOFF		0x1d	// Reserve MegaSD OFF (warm reset to go: 0xfd)
#define OCM_SMART_MegaSDON		0x1e	// Reserve MegaSD ON (warm reset to go: 0xfd)
/*
0x1f	// Set MegaSD Blink OFF
0x20	// Set MegaSD Blink ON (default)
0x21	// Set Lights Mode OFF w/ Auto LEDs Control (default)
0x22	// Set Lights Mode ON + Red Led OFF
0x23	// Set Lights Mode ON + Red Led ON
*/
#define OCM_SMART_AudioPreset1	0x24	// Internal Audio Preset #1 "Mute Sound"
#define OCM_SMART_AudioPreset2	0x25	// Internal Audio Preset #2 "Middle Sound"
#define OCM_SMART_AudioPreset3	0x26	// Internal Audio Preset #3 "High Sound" (default)
/*
0x27	// Set CMT OFF (default) (disabled w/ MSXtR BIOS) (n/a on SM-X / SX-2)
0x28	// Set CMT ON (needs a cassette recorder) (disabled w/ MSXtR BIOS) (n/a on SM-X / SX-2)
0x29	// Lock Turbo Toggles
0x2a	// Unlock Turbo Toggles
0x2b	// Lock Display Toggles
0x2c	// Unlock Display Toggles
0x2d	// Lock Audio Mixer & SCRLK Toggles
0x2e	// Unlock Audio Mixer & SCRLK Toggles
0x2f	// Lock Slot-1 Toggles
0x30	// Unlock Slot-1 Toggles
0x31	// Lock Slot-2 Toggles
0x32	// Unlock Slot-2 Toggles
0x33	// Lock Slot-1 & Slot-2 Toggles
0x34	// Unlock Slot-1 & Slot-2 Toggles
0x35	// Lock Hard Reset Key
0x36	// Unlock Hard Reset Key
0x37	// Lock Mapper Toggle
0x38	// Unlock Mapper Toggle
0x39	// Lock MegaSD Toggle
0x3a	// Unlock MegaSD Toggle
0x3b	// Lock All Toggles
0x3c	// Unlock All Toggles (default)
*/
#define OCM_SMART_PseudSterOFF	0x3d	// Set Pseudo-Stereo OFF (default)
#define OCM_SMART_PseudSterON	0x3e	// Set Pseudo-Stereo ON (needs an external sound cartridge)

#define OCM_SMART_ExtBusCPU		0x3f	// Sync External Bus Clock to CPU Speed (default)
#define OCM_SMART_ExtBus358		0x40	// Set External Bus Clock 3.58 MHz

#define OCM_SMART_TurboPana		0x41	// Set Turbo Pana 5.37MHz mode

#define OCM_SMART_RightInvAud0	0x42	// Set Right Inverse Audio OFF (default)
#define OCM_SMART_RightInvAud1	0x43	// Set Right Inverse Audio ON

#define OCM_SMART_AudioPreset4	0x44	// Internal Audio Preset #4 "Emphasis PSG Sound" (since IO7)
#define OCM_SMART_AudioPreset5	0x45	// Internal Audio Preset #5 "Emphasis SCC-I Sound" (since IO7)
#define OCM_SMART_AudioPreset6	0x46	// Internal Audio Preset #6 "Emphasis OPLL Sound" (since IO7)

#define OCM_SMART_VertOffset16	0x47	// Vertical Offset 16 (useful for Ark-A-Noah)
#define OCM_SMART_VertOffset17	0x48	// Vertical Offset 17
#define OCM_SMART_VertOffset18	0x49	// Vertical Offset 18
#define OCM_SMART_VertOffset19	0x4a	// Vertical Offset 19 (default)
#define OCM_SMART_VertOffset20	0x4b	// Vertical Offset 20
#define OCM_SMART_VertOffset21	0x4c	// Vertical Offset 21
#define OCM_SMART_VertOffset22	0x4d	// Vertical Offset 22
#define OCM_SMART_VertOffset23	0x4e	// Vertical Offset 23
#define OCM_SMART_VertOffset24	0x4f	// Vertical Offset 24 (useful for Space Manbow)

#define OCM_SMART_Scanlines00	0x50	// Set VGA Scanlines 0% (default) (only for SM-X / SX-2)
#define OCM_SMART_Scanlines25	0x51	// Set VGA Scanlines 25% (only for SM-X / SX-2)
#define OCM_SMART_Scanlines50	0x52	// Set VGA Scanlines 50% (only for SM-X / SX-2)
#define OCM_SMART_Scanlines75	0x53	// Set VGA Scanlines 75% (only for SM-X / SX-2)

#define OCM_SMART_IntPSG2OFF	0x54	// Set Internal PSG2 OFF (default) (only for SM-X / SX-2)
#define OCM_SMART_IntPSG2ON		0x55	// Set Internal PSG2 ON (this second PSG acts as an external PSG) (only for SM-X / SX-2)

#define OCM_SMART_Mapper4MbOFF	0x56	// Set Extra-Mapper 4096KB OFF (default)
#define OCM_SMART_Mapper4MbON	0x57	// Set Extra-Mapper 4096KB ON (only available if SDRAM > 8MB)
/*
0x58	// Set Extended MegaROM Reading OFF (default for compatibility)
0x59	// Set Extended MegaROM Reading ON (ASCII-8K/16K max size playable)
...
0x7f	// Pixel Ratio 1:1 for LED Display (default is 0) (range 0-7) (60Hz only)
*/
#define OCM_SMART_NullCommand	0x80	// Null Command (useful for programming)

#define OCM_SMART_LegacyVGA		0x81	// Assign Legacy Output to VGA
#define OCM_SMART_LegacyVGAplus	0x82	// Assign Legacy Output to VGA+ (default)
/*
0x83	// Set Internal Slot-1 Linear OFF (default)
0x84	// Set Internal Slot-1 Linear ON (requires SCC-I preset)
0x85	// Set Internal Slot-2 Linear OFF (default)
0x86	// Set Internal Slot-2 Linear ON (requires SCC-I or ASCII-8K/16K preset)
*/
#define OCM_SMART_OPL3OFF		0x87	// Set Internal OPL3 OFF (default) (only for SM-X / SX-2)
#define OCM_SMART_OPL3ON		0x88	// Set Internal OPL3 ON (only for SM-X / SX-2)
/*
0x89-0x8f	// Reserved (Ducasp) (only for SM-X / SX-2)
...
*/
#define OCM_SMART_MasterVol0	0xb0	// Set Master Volume 0
#define OCM_SMART_MasterVol1	0xb1	// Set Master Volume 1
#define OCM_SMART_MasterVol2	0xb2	// Set Master Volume 2
#define OCM_SMART_MasterVol3	0xb3	// Set Master Volume 3
#define OCM_SMART_MasterVol4	0xb4	// Set Master Volume 4
#define OCM_SMART_MasterVol5	0xb5	// Set Master Volume 5
#define OCM_SMART_MasterVol6	0xb6	// Set Master Volume 6
#define OCM_SMART_MasterVol7	0xb7	// Set Master Volume 7 (default)

#define OCM_SMART_PSGVol0		0xb8	// Set PSG Volume 0
#define OCM_SMART_PSGVol1		0xb9	// Set PSG Volume 1
#define OCM_SMART_PSGVol2		0xba	// Set PSG Volume 2
#define OCM_SMART_PSGVol3		0xbb	// Set PSG Volume 3
#define OCM_SMART_PSGVol4		0xbc	// Set PSG Volume 4 (default)
#define OCM_SMART_PSGVol5		0xbd	// Set PSG Volume 5
#define OCM_SMART_PSGVol6		0xbe	// Set PSG Volume 6
#define OCM_SMART_PSGVol7		0xbf	// Set PSG Volume 7

#define OCM_SMART_SCCIVol0		0xc0	// Set SCC-I Volume 0
#define OCM_SMART_SCCIVol1		0xc1	// Set SCC-I Volume 1
#define OCM_SMART_SCCIVol2		0xc2	// Set SCC-I Volume 2
#define OCM_SMART_SCCIVol3		0xc3	// Set SCC-I Volume 3
#define OCM_SMART_SCCIVol4		0xc4	// Set SCC-I Volume 4 (default)
#define OCM_SMART_SCCIVol5		0xc5	// Set SCC-I Volume 5
#define OCM_SMART_SCCIVol6		0xc6	// Set SCC-I Volume 6
#define OCM_SMART_SCCIVol7		0xc7	// Set SCC-I Volume 7

#define OCM_SMART_OPLLVol0		0xc8	// Set OPLL Volume 0
#define OCM_SMART_OPLLVol1		0xc9	// Set OPLL Volume 1
#define OCM_SMART_OPLLVol2		0xca	// Set OPLL Volume 2
#define OCM_SMART_OPLLVol3		0xcb	// Set OPLL Volume 3
#define OCM_SMART_OPLLVol4		0xcc	// Set OPLL Volume 4 (default)
#define OCM_SMART_OPLLVol5		0xcd	// Set OPLL Volume 5
#define OCM_SMART_OPLLVol6		0xce	// Set OPLL Volume 6
#define OCM_SMART_OPLLVol7		0xcf	// Set OPLL Volume 7

#define OCM_SMART_ForceNTSC		0xd0	// Force NTSC Mode
#define OCM_SMART_VideoAuto		0xd1	// Standard NTSC/PAL Mode (bound by Control Register 9) (default)
#define OCM_SMART_ForcePAL		0xd2	// Force PAL Mode
/*
0xd3	// Restore Default Keyboard Layout
0xd4	// Null Command (reserved)
*/
#define OCM_SMART_RestoreTurbo	0xd5	// Restore Default Turbo Mode

#define OCM_SMART_CenterYJKOFF	0xd6	// Set Centering YJK Modes/R25 Mask OFF (default)
#define OCM_SMART_CenterYJKON	0xd7	// Set Centering YJK Modes/R25 Mask ON
/*
...
0xf8	// Reserve OCM-BIOS Reloading (cold reset or warm reset to go)
0xf9	// Reserve Slot-0 Primary Mode (warm reset to go) (internal OPLL disabled) (C-BIOS support)
0xfa	// Reserve System Logo ON (warm reset only)
*/
#define OCM_SMART_ColdReset		0xfb	// Cold Reset
/*
0xfc	// Warm Reset w/ Mapper 2048KB (RAM size 6144KB if Extra-Mapper is ON)
*/
#define OCM_SMART_WarmReset		0xfd	// Warm Reset
/*
0xfe	// Warm Reset w/ Mapper 4096KB (RAM size 8192KB if Extra-Mapper is ON)
0xff	// Restore All Default + Reserve Default Mapper & MegaSD
*/


// ========================================================
//  Functions

bool ocm_detectDevice(DeviceId_t devId) __z88dk_fastcall;
uint8_t ocm_getPortValue(uint8_t port) __z88dk_fastcall;
uint16_t ocm_getDynamicPortValue(uint8_t index) __z88dk_fastcall;
bool ocm_sendSmartCmd(uint8_t cmd) __z88dk_fastcall;
