#include <nds.h>

#include <stdio.h>
#include "font.h"

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	irqInit();
	irqEnable(IRQ_VBLANK);
	unsigned int i;
	const int char_base = 0;
	const int screen_base = 20;
	videoSetMode(0);	

	// 16bit tilemaps for rot scale text backgrounds require extended palettes
	videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_BG_EXT_PALETTE);	
	vramSetBankC(VRAM_C_SUB_BG); 

	// rot scale backgrounds have a different size code
	SUB_BG3_CR = BG_TILE_BASE(char_base) | BG_MAP_BASE(screen_base) | ROTBG_SIZE_256x256;
	
	u16* sub_tile = (u16*)CHAR_BASE_BLOCK_SUB(char_base);
	u16* sub_map = (u16*)SCREEN_BASE_BLOCK_SUB(screen_base);

	//95 and 32 show how many characters there are and 32 shows which ASCII character to start, respectively
	//95 is the smaller set of ACSII characters. It usually will start with 32
	consoleInit((u16*)fontData, sub_tile, 95, 32, sub_map, CONSOLE_USE_COLOR255, 8);
    
	//Load the Font Data and Palette stuff here
	for(i = 0; i < fontDataSize; ++i) {
		sub_tile[i] = fontData[i];
	}

	// extended palettes are written with bank mapped to lcd
	vramSetBankH(VRAM_H_LCD); 

	// each background has it's own set of 16 256 color palettes
	for(i = 0; i < fontPaletteSize; ++i) {
		VRAM_H_EXT_PALETTE[3][0][i]= fontPalette[i];
	}

	// map bank to extended palette after writing data
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE); 

	iprintf("Custom Font Demo\n");
	iprintf("   by Poffy\n");
	iprintf("modified by WinterMute\n");
	iprintf("for libnds examples\n");

	//scale is fixed point
	s16 scaleX = 1 << 8, scaleY = 1 << 8;

	s16 scrollX = 128 , scrollY = 96;

	//this is the screen pixel that the image will rotate about
	s16 rcX = 128, rcY = 96;

	unsigned int angle = 0;

	while(1) {
		scanKeys();
		u32 keys = keysHeld();

		if ( keys & KEY_L ) angle++; 
		if ( keys & KEY_R ) angle--;

		if ( keys & KEY_LEFT ) scrollX++;
		if ( keys & KEY_RIGHT ) scrollX--;
		if ( keys & KEY_UP ) scrollY++;
		if ( keys & KEY_DOWN ) scrollY--;

		if ( keys & KEY_A ) scaleX++;
		if ( keys & KEY_B ) scaleX--;

		if( keys & KEY_X ) scaleY++;
		if( keys & KEY_Y ) scaleY--;

		// wrap angle
		angle &= 0x1ff;

		// Compute sin and cos
		s16 angleSin = SIN[angle] >> 4;
		s16 angleCos = COS[angle] >> 4;
 
		swiWaitForVBlank();

		// Set the background registers
		SUB_BG3_XDX = ( angleCos * scaleX ) >> 8;
		SUB_BG3_XDY = (-angleSin * scaleX ) >> 8;
		SUB_BG3_YDX = ( angleSin * scaleY ) >> 8;
		SUB_BG3_YDY = ( angleCos * scaleY ) >> 8;

		SUB_BG3_CX = (scrollX<<8) - rcX * ( angleCos - angleSin);
		SUB_BG3_CY = (scrollY<<8) - rcY * ( angleSin + angleCos);

	}

}