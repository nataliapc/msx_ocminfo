/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.

	- - - - - - - - - - - - - - - - - - - - - - - -

	The file OCMINFO.CFG use the following struct:

	--- ProfileHeader_t
	--- ProfileHeaderData_t
	--- ProfileItem_t 0
	--- ProfileItem_t 1
		...
	--- ProfileItem_t n

*/
#pragma once
#include <stdint.h>
#include <stdbool.h>


// ========================================================
// Defines

#define PROF_MAGIC		{ 0x464f5250 }	//"PROF"
#define PROF_REV		2


// ========================================================
// Struct & Enums

typedef struct {
	uint32_t magic;					// "PROF" chars
	uint8_t  revision;				// Current Profiles file revision (1)
	uint16_t headerLength;			// SizeOf(ProfileHeaderData)
	uint8_t  checksum;				// HeaderData + Items checksum
} ProfileHeader_t;

typedef struct {
	uint8_t  itemsCount;			// Number of profiles stored
	uint16_t itemLength;			// Length of each profile item
	bool     muteSound;				// Mute sound (default: false)
} ProfileHeaderData_t;

typedef struct {
	char     description[60];		// AsciiZ
	uint16_t modifYear;				// Modification date: Year
	uint8_t  modifMonth;			// Modification date: Month
	uint8_t  modifDay;				// Modification date: Day
	uint8_t  cmd[40];				// SetSmart commands
	uint8_t  reserved[24];			// Reserved
} ProfileItem_t;


// ========================================================
// Functions

void profile_init();
void profile_release();
bool profile_loadFile();
bool profile_saveFile();
ProfileHeader_t* profile_getHeader();
ProfileHeaderData_t* profile_getHeaderData();
uint8_t profile_newItem();
ProfileItem_t* profile_getItem(uint8_t idx);
bool profile_updateItem(uint8_t idx);
bool profile_deleteItem(uint8_t idx);
