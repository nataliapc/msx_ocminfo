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
#include <stdint.h>
#include <stdbool.h>


// ========================================================
// Defines

#define PROF_MAGIC	{ 0x464f5250 }	//"PROF"
#define PROF_REV	1


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
} ProfileHeaderData_t;

typedef struct {
	char    description[60];		// StringZ
	uint8_t modifYear;				// Modification date: Year
	uint8_t modifMonth;				// Modification date: Month
	uint8_t modifDay;				// Modification date: Day
	uint8_t cmd[40];				// SetSmart commands
} ProfileItem_t;


// ========================================================
// Functions

bool profile_loadFile();
bool profile_saveFile();
ProfileHeader_t* profile_getHeader();
ProfileHeaderData_t* profile_getHeaderData();
ProfileItem_t* profile_getItem(uint8_t idx);
