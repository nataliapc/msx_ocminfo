/*
	Copyright (c) 2022 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma opt_code_size
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dialogs.h"
#include "conio.h"
#include "heap.h"


// ========================================================
// Private functions
static void _fillBlink(uint8_t x, uint8_t y, uint8_t lines, uint8_t len, bool enabled)
{
	while (lines--) {
		textblink(x, y++, len, enabled);
	}
}

// ========================================================
uint8_t showDialog(Dialog_t *dlg)
{
	uint8_t dlgWidth, dlgHeight;
	uint8_t numLines, maxLineLen = 0;
	uint8_t linesLen[DLG_MAX_TXT];
	uint8_t numBtn, totalBtnLen = 0;
	uint8_t btnLen[DLG_MAX_BTN], btnX[DLG_MAX_BTN];
	uint8_t selectedBtn = dlg->defaultButton;
	uint8_t key, i, auxX, auxY;
	bool end = false;

	// Calculate dialog sizes
	for (numLines=0; ; numLines++) {
		if (dlg->text[numLines] == NULL) break;
		linesLen[numLines] = strlen(dlg->text[numLines]);
		if (linesLen[numLines] > maxLineLen) maxLineLen = linesLen[numLines];
	}
	for (numBtn=0; ; numBtn++) {
		if (dlg->buttons[numBtn] == NULL) break;
		btnLen[numBtn] = strlen(dlg->buttons[numBtn]);
		totalBtnLen += btnLen[numBtn] + 1;
	}
	if (numBtn) totalBtnLen--;

	dlgWidth = maxLineLen;
	if (dlgWidth < totalBtnLen) dlgWidth = totalBtnLen;
	dlgWidth += 6;	// Add borders & margin

	dlgHeight = numLines + (numBtn ? 2 : 0) + 2;	// lines + (margin + buttons) + borders

	// Calculate dialog position
	uint8_t dx1 = dlg->posX,
			dy1 = dlg->posY,
			dx2, dy2;
	if (!dx1) dx1 = (80 - dlgWidth) / 2 + 1;
	if (!dy1) dy1 = (24 - dlgHeight) / 2 + 1;
	dx2 = dx1 + dlgWidth - 1;
	dy2 = dy1 + dlgHeight - 1;
	uint16_t dlgBytes = (dx2 - dx1 + 1) * (dy2 - dy1 + 1);
	char *scrBackup = malloc(dlgBytes);

	// Draw dialog
	gettext(dx1,dy1, dx2,dy2, scrBackup);				// Backup rectangle chars

	memset(heap_top, ' ', dlgBytes);					// Clear rectangle
	puttext(dx1,dy1, dx2,dy2, heap_top);

	drawFrame(dx1+1,dy1, dx2-1,dy2);					// Draw frame
	_fillBlink(dx1, dy1, dlgHeight, dx2-dx1+1, true);
	for (i = 0; i<numLines; i++) {
		putlinexy(dx1 + (dx2-dx1-linesLen[i])/2 + 1, dy1+i+1, linesLen[i], dlg->text[i]);
	}

	auxX = dx1 + (dx2-dx1-totalBtnLen)/2 + 1;
	auxY = dy1 + numLines + 2;
	for (i = 0; i<numBtn; i++) {
		putlinexy(auxX, auxY, btnLen[i], dlg->buttons[i]);
		btnX[i] = auxX;
		auxX += btnLen[i];
		auxX++;
	}

	// Dialog loop
	textblink(btnX[selectedBtn],auxY, btnLen[selectedBtn], false);
	while (!end) {
		while (!kbhit()) { ASM_EI; ASM_HALT; }
		textblink(btnX[selectedBtn],auxY, btnLen[selectedBtn], true);
		key = getch();
		if (dlg->buttons[0] == NULL) {
			end++;
		} else
		if (key == KEY_LEFT || key == KEY_UP) {
			selectedBtn = (selectedBtn ? --selectedBtn : numBtn-1);
		} else
		if (key == KEY_RIGHT || key == KEY_DOWN || key ==KEY_TAB) {
			selectedBtn++;
			selectedBtn %= numBtn;
		} else
		if (key == KEY_ESC) {
			selectedBtn = dlg->cancelButton;
			end++;
		} else
		if (key == KEY_ENTER || key == KEY_SELECT || key == KEY_SPACE) {
			end++;
		}
		textblink(btnX[selectedBtn],auxY, btnLen[selectedBtn], false);
		varPUTPNT = varGETPNT;
		varREPCNT = 0;
	}

	// Restore background
	ASM_EI;
	ASM_HALT;
	_fillBlink(dx1, dy1, dlgHeight, dx2-dx1+1, false);
	puttext(dx1,dy1, dx2,dy2, scrBackup);
	free(dlgBytes);

	return selectedBtn;
}