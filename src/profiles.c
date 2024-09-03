/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma opt_code_size
#include <stdint.h>
#include "dos.h"
#include "msx_const.h"
#include "heap.h"
#include "profiles.h"


// ========================================================
// Private variables

static char *_filename = "A:\\OCMINFO.CFG";

static ProfileHeader_t _header = { PROF_MAGIC, PROF_REV, sizeof(ProfileHeaderData_t), 0x00 };
static ProfileHeaderData_t _headerData = { 0x00 };
static ProfileItem_t *_profiles = NULL;


// ========================================================
// Private functions

static char _getBootDrive()
{
	// Get the first drive available (boot drive)
	uint8_t drive = 'A';
	RETW drives = availableDrives();
	if (!drives) return '\0';
	while (drives && !(drives & 1)) {
		drives >>= 1;
	}
	return drive;
}

static uint8_t _calculateChecksum()
{
	uint8_t calculatedChecksum = 0;
	uint16_t i;
	// Calculate checksum for header data
	for (i = 0; i < sizeof(ProfileHeaderData_t); i++) {
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

bool profile_loadFile()
{
	_filename[0] = _getBootDrive();
	if (*_filename == '\0') return false;

	FILEH fh = dos2_fopen(_filename, O_RDONLY);
	if (fh >= ERR_FIRST)
		return false;

	// Read header
	if (dos2_fread((char*)&_header, sizeof(ProfileHeader_t), fh) != sizeof(ProfileHeader_t))
		return false;

	// Read header data
	if (dos2_fread((char*)&_headerData, _header.headerLength, fh) != _header.headerLength)
		return false;

	// Read profile items
	uint16_t profilesTotalLen = sizeof(ProfileItem_t) * _headerData.itemsCount;
	_profiles = malloc(profilesTotalLen);
	if (!_profiles) return false;
	if (dos2_fread((char*)_profiles, profilesTotalLen, fh) != profilesTotalLen)
		goto release_memory_and_fail;

	if (!_isValidChecksum())
		goto release_memory_and_fail;

	dos2_fclose(fh);
	return true;

release_memory_and_fail:
	free(profilesTotalLen);
	return false;
}

bool profile_saveFile()
{
	_filename[0] = _getBootDrive();
	if (*_filename == '\0') return false;

	FILEH fh = dos2_fcreate(_filename, O_WRONLY, ATTR_ARCHIVE);
	if (fh >= ERR_FIRST)
		return false;

	// Write header
	_header.checksum = _calculateChecksum();
	if (dos2_fwrite((char*)&_header, sizeof(ProfileHeader_t), fh) != sizeof(ProfileHeader_t))
		return false;

	// Write header data
	if (dos2_fwrite((char*)&_headerData, _header.headerLength, fh) != _header.headerLength)
		return false;

	// Write profile items
	uint16_t profilesTotalLen = sizeof(ProfileItem_t) * _headerData.itemsCount;
	if (dos2_fwrite((char*)_profiles, profilesTotalLen, fh) != profilesTotalLen)
		return false;

	dos2_fclose(fh);
	return true;
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
