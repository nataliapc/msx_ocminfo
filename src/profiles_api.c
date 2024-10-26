/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma opt_code_size
#include <stdint.h>
#include <string.h>
#include "msx_const.h"
#include "conio.h"
#include "dos.h"
#include "heap.h"
#include "profiles_api.h"


// ========================================================
// Private variables

static char *filename = "A:\\OCMINFO.CFG";
static void *original_heaptop = NULL;

static ProfileHeader_t _header = { PROF_MAGIC, PROF_REV, sizeof(ProfileHeaderData_t), 0x00 };
static ProfileHeaderData_t _headerData = { 0, sizeof(ProfileItem_t), false };
static ProfileItem_t *_profiles = NULL;
static SYSTEMDATE_t date;


// ========================================================
// Private & external functions

extern void getPanelsCmds(uint8_t *cmd);

static bool _setFilenameWithBootDrive()
{
	// Set the first drive available (boot drive)
	RETW drives = availableDrives();
	if (!drives) return false;
	*filename = 'A';
	while (drives && !(drives & 1)) {
		drives >>= 1;
		*filename++;
	}
	return true;
}

static uint8_t _calculateChecksum()
{
	uint8_t calculatedChecksum = 0;
	uint16_t i;
	// Calculate checksum for header data
	for (i = 0; i < _header.headerLength; i++) {
		calculatedChecksum += ((uint8_t*)&_headerData)[i];
	}
	// Calculate checksum for profile items
	for (i = 0; i < _headerData.itemsCount * sizeof(ProfileItem_t); i++) {
		calculatedChecksum += ((uint8_t*)_profiles)[i];
	}
	return calculatedChecksum;
}

static bool _isValidChecksum()
{
	// Compare calculated checksum with stored checksum
	return (_calculateChecksum() == _header.checksum);
}


// ========================================================
// Functions

void profile_init()
{
	profile_release();
	_header.headerLength = sizeof(ProfileHeaderData_t);
	_headerData.itemsCount = 0;
	_headerData.itemLength = sizeof(ProfileItem_t);
	_header.checksum = _calculateChecksum();
	
	original_heaptop = heap_top;
	_profiles = (ProfileItem_t*)heap_top;
}

inline void profile_release()
{
	if (original_heaptop) heap_top = original_heaptop;
	original_heaptop = NULL;
}

bool profile_loadFile()
{
	bool result = false;
	uint16_t profilesTotalLen = 0;

	if (!_setFilenameWithBootDrive()) return false;

	FILEH fh = dos2_fopen(filename, O_RDONLY);
	if (fh >= ERR_FIRST)
		goto load_end;

	// Read header
	if (dos2_fread((char*)&_header, sizeof(ProfileHeader_t), fh) != sizeof(ProfileHeader_t))
		goto load_end;

	// Read header data
	if (dos2_fread((char*)&_headerData, _header.headerLength, fh) != _header.headerLength)
		goto load_end;

	// Read profile items
	profilesTotalLen = sizeof(ProfileItem_t) * _headerData.itemsCount;
	_profiles = malloc(profilesTotalLen);
	if (!_profiles || dos2_fread((char*)_profiles, profilesTotalLen, fh) != profilesTotalLen)
		goto load_release;

	if (!_isValidChecksum())
		goto load_release;

	result = true;
	goto load_end;

load_release:
	if (profilesTotalLen) profile_init();

load_end:
	dos2_fclose(fh);
	return result;
}

bool profile_saveFile()
{
	bool result = false;

	if (!_setFilenameWithBootDrive()) return false;

	ERRB err = dos2_remove(filename);
	FILEH fh = dos2_fcreate(filename, O_WRONLY, ATTR_ARCHIVE|ATTR_HIDDEN);
	if (fh >= ERR_FIRST) goto save_fail;

	// Write header
	_header.revision = PROF_REV;
	_header.headerLength = sizeof(ProfileHeaderData_t);
	_header.checksum = _calculateChecksum();
	if (dos2_fwrite((char*)&_header, sizeof(ProfileHeader_t), fh) != sizeof(ProfileHeader_t))
		goto save_fail;

	// Write header data
	if (dos2_fwrite((char*)&_headerData, _header.headerLength, fh) != _header.headerLength)
		goto save_fail;

	// Write profile items
	uint16_t profilesTotalLen = sizeof(ProfileItem_t) * _headerData.itemsCount;
	if (dos2_fwrite((char*)_profiles, profilesTotalLen, fh) != profilesTotalLen)
		goto save_fail;

	result = true;
save_fail:
	dos2_fclose(fh);
	return result;
}

inline ProfileHeader_t* profile_getHeader()
{
	return &_header;
}

inline ProfileHeaderData_t* profile_getHeaderData()
{
	return &_headerData;
}

ProfileItem_t* profile_getItem(uint8_t idx)
{
	if (idx >= _headerData.itemsCount) return NULL;
	return &_profiles[idx];
}

uint8_t profile_newItem()
{
	// Allocate & clean new profile
	ProfileItem_t *newProfile = malloc(sizeof(ProfileItem_t));
	memset(newProfile, 0, sizeof(ProfileItem_t));

	// Set values
	getSystemDate(&date);
	getPanelsCmds(newProfile->cmd);
	newProfile->cmd[0];
	newProfile->modifYear = date.year;
	newProfile->modifMonth = date.month;
	newProfile->modifDay = date.day;

	_headerData.itemsCount++;
	return newProfile - _profiles;
}

bool profile_updateItem(uint8_t idx)
{
	// Get profile to update
	ProfileItem_t *profile = profile_getItem(idx);
	if (profile == NULL) return false;

	// Update values
	getSystemDate(&date);
	getPanelsCmds(profile->cmd);
	profile->modifYear = date.year;
	profile->modifMonth = date.month;
	profile->modifDay = date.day;

	return true;
}

bool profile_deleteItem(uint8_t idx)
{
	if (_headerData.itemsCount) {
		if (idx+1 < _headerData.itemsCount) {
			memcpy(
				&_profiles[idx],
				&_profiles[idx+1],
				(_headerData.itemsCount-idx-1)*sizeof(ProfileItem_t));
		}
		_headerData.itemsCount--;
		free(sizeof(ProfileItem_t));
	}
}
