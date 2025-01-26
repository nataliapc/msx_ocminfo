/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>


void waitVBLANK(void);
void die(const char *s, ...);
void exit(void);

uint8_t getRomByte(uint16_t address) __sdcccall(1);

void basic_play(void *parameters) __sdcccall(1);


#define MODE_ANK		0
#define MODE_KANJI0		1
#define MODE_KANJI1		2
#define MODE_KANJI2		3
#define MODE_KANJI3		4
bool detectKanjiDriver() __z88dk_fastcall;
char getKanjiMode() __sdcccall(1);
void setKanjiMode(uint8_t mode) __z88dk_fastcall;


typedef union {
	uint16_t raw;
	struct {
		// ROW E (14)
		unsigned unused0_6: 7;
		unsigned scrLk : 1;		// Scroll Lock key
		// ROW F (15)
		unsigned f12: 1;
		unsigned f11: 1;
		unsigned f10: 1;
		unsigned f9:  1;
		unsigned pgDn:  1;		// Page Down key
		unsigned pgUp:  1;		// Page Up key
		unsigned prtSc: 1;		// Print Screen key
		unsigned unused15: 1;
	};
} ExtraKeysOCM_t;
ExtraKeysOCM_t getExtraKeysOCM() __naked __sdcccall(1);


#endif//__UTILS_H__
