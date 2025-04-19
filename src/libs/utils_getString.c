#include "heap.h"
#include "utils.h"
#include "strings_index.h"

extern const unsigned char out_strings_bin_zx0[];
static char *strings_table = 0;


/**
 * @brief Decompresses the ZX0-compressed strings table into RAM.
 * Must be called before using getString().
 */
void stringsInit()
{
	/**
	 * Allocate memory for the uncompressed strings table and decompress.
	 * The memory is persistent for the program's lifetime.
	 */
	if (!strings_table)
	{
		strings_table = (char *)malloc(STRINGS_BIN_SIZE);
		// dzx0_standard(src, dest)
		dzx0_standard(out_strings_bin_zx0, strings_table);
	}
}

/**
 * @brief Returns a pointer to the string at the given offset.
 * @param pos Offset in the uncompressed strings table (use macro from strings_index.h)
 * @return Pointer to null-terminated string, or 0 if not initialized.
 */
char *getString(uint16_t pos)
{
	if (!strings_table)
		return 0;
	return (char *)(strings_table + pos);
}