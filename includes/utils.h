#pragma once

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>


void die(const char *s, ...);
void exit(void);

uint8_t getRomByte(uint16_t address) __sdcccall(1);


#define MODE_ANK		0
#define MODE_KANJI0		1
#define MODE_KANJI1		2
#define MODE_KANJI2		3
#define MODE_KANJI3		4
bool detectKanjiDriver() __z88dk_fastcall;
char getKanjiMode() __sdcccall(1);
void setKanjiMode(uint8_t mode) __z88dk_fastcall;


#endif//__UTILS_H__
