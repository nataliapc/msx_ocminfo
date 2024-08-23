#pragma once

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>


void die(const char *s, ...);
void exit(void);

uint8_t getRomByte(uint16_t address) __sdcccall(1);


#endif//__UTILS_H__
