#include "GBGPU.h"


// Register addresses
#define LCDCbyte	0xFF40
#define STATbyte	0xFF41
#define SCYbyte		0xFF42
#define SCXbyte		0xFF43
#define LYbyte		0xFF44
#define LYCbyte		0xFF45
#define DMAbyte		0xFF46
#define BGPbyte		0xFF47
#define OBP0byte	0xFF48
#define OBP1byte	0xFF49
#define WYbyte		0xFF4A
#define WXbyte		0xFF4B
// Interrupt address and bytes
#define IFbyte      0xFF0F	// Might not be necessary
#define vblankint	0x01
#define LCDCint		0x02
// STAT Interrupt bits
#define mode0		0x8
#define mode1		0x10
#define mode2		0x20
#define coincidence	0x40


GBGPU::GBGPU(Interrupts &interrupts, Cartridge* gbCart, WRAM &wram, bool CGBmode) : interrupts(&interrupts), gbCart(gbCart),
	wram(&wram), CGBmode(CGBmode)
{
}


GBGPU::~GBGPU()
{
}

void GBGPU::sendData(uint16_t address, uint8_t data) {
	// TODO: these can't always be written to
	if (address >= 0x8000 && address <= 0x9FFF) {
		VRAM[(address & 0x1FFF)|(VRAMBank << 13)] = data;
	}
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		OAM[address & 0xFF] = data;
	}
	// Register write
	else if (address >= 0xFF40 && address <= 0xFF4B) {
		switch (address) {
			case LCDCbyte:
				LCDC = data;
				break;
			case STATbyte:
				//STAT = data;
				STAT = (STAT & 0x7) | (data & 0xF8);
				break;
			case SCYbyte:
				SCY = data;
				break;
			case SCXbyte:
				SCX = data;
				break;
			case LYbyte:
				LY = 0;
				windowLineCounter = 0;
				break;
			case LYCbyte:
				LYC = data;
				break;
			case DMAbyte: {
				// Handled in Memory
				break;
			}
			case BGPbyte:
				BGP = data;
				break;
			case OBP0byte:
				OBP0 = data;
				break;
			case OBP1byte:
				OBP1 = data;
				break;
			case WYbyte:
				WY = data;
				break;
			case WXbyte:
				WX = data;
				break;
		}
	}
	// VRAM Bank
	else if (address == 0xFF4F) {
		VRAMBank = data;
	}
	// CGB HDMA
	else if (address >= 0xFF51 && address <= 0xFF55) {
		switch (address) {
		case 0xFF51:
			HDMAsrc = (HDMAsrc & 0xFF) | (data << 8);
			break;
		case 0xFF52:
			HDMAsrc = (HDMAsrc & 0xFF00) | (data & 0xF0);
			break;
		case 0xFF53:
			HDMAdst = (HDMAdst & 0xFF) | (((data & 0x1F)| 0x80) << 8);
			break;
		case 0xFF54:
			HDMAdst = (HDMAdst & 0xFF00) | (data & 0xF0);
			break;
		case 0xFF55:
			// START DMA
			HDMAlen = data;
			if ((HDMAlen & 0x80) != 0x80 && !HDMAActive) {
				// Instant DMA
				for (int i = 0; i <= (HDMAlen & 0x7F); i++) {
					HDMA();
				}
				HDMAlen = 0;
				HDMAActive = false;
			}
			else if ((HDMAlen & 0x80) != 0x80 && HDMAActive) {
				HDMAActive = false;
			}
			else {
				HDMAActive = true;
			}
			break;
		}
	}
	// CGB Palette Data
	else if (address >= 0xFF68 && address <= 0xFF6B) {
		switch (address) {
			case 0xFF68:
				cgbBGPaletteIndex = data;
				break;
			case 0xFF69:
				cgbBGPalette[cgbBGPaletteIndex & 0x3F] = data;
				if ((cgbBGPaletteIndex & 0x80) == 0x80) {
					cgbBGPaletteIndex = (cgbBGPaletteIndex + 1) & 0xBF;
				}
				break;
			case 0xFF6A:
				cgbSPRPaletteIndex = data;
				break;
			case 0xFF6B:
				cgbSPRPalette[cgbSPRPaletteIndex & 0x3F] = data;
				if ((cgbSPRPaletteIndex & 0x80) == 0x80) {
					cgbSPRPaletteIndex = (cgbSPRPaletteIndex + 1) & 0xBF;
				}
				break;
		}
	}
}

uint8_t GBGPU::receiveData(uint16_t address) {
	uint8_t returnData = 0;

	if (address >= 0x8000 && address <= 0x9FFF) {
		returnData = VRAM[(address & 0x1FFF) | (VRAMBank << 13)];
	}
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		returnData = OAM[address & 0xFF];
	}
	// Register write
	else if (address >= 0xFF40 && address <= 0xFF4B) {
		switch (address) {
			case LCDCbyte:
				returnData = LCDC;
				break;
			case STATbyte:
				returnData = STAT;
				break;
			case SCYbyte:
				returnData = SCY;
				break;
			case SCXbyte:
				returnData = SCX;
				break;
			case LYbyte:
				returnData = LY;
				break;
			case LYCbyte:
				returnData = LYC;
				break;
			case DMAbyte:
				// DMA can't be read
				break;
			case BGPbyte:
				returnData = BGP;
				break;
			case OBP0byte:
				returnData = OBP0;
				break;
			case OBP1byte:
				returnData = OBP1;
				break;
			case WYbyte:
				returnData = WY;
				break;
			case WXbyte:
				returnData = WX;
				break;
		}
	}
	// VRAM Bank
	else if (address == 0xFF4F) {
		returnData = VRAMBank;
	}
	// CGB HDMA
	else if (address >= 0xFF51 && address <= 0xFF55) {
		switch (address) {
		case 0xFF51:
			returnData = HDMAsrc >> 8;
			break;
		case 0xFF52:
			returnData = HDMAsrc & 0xFF;
			break;
		case 0xFF53:
			returnData = HDMAdst >> 8;
			break;
		case 0xFF54:
			returnData = HDMAdst & 0xFF;
			break;
		case 0xFF55:
			returnData = (HDMAlen & 0x7F) | (HDMAActive ? 0 : 1) << 7;
			break;
		}
	}
	// CGB Palette Data
	else if (address >= 0xFF68 && address <= 0xFF6B) {
		switch (address) {
			case 0xFF68:
				returnData = cgbBGPaletteIndex;
				break;
			case 0xFF69:
				returnData = cgbBGPalette[cgbBGPaletteIndex & 0x3F];
				break;
			case 0xFF6A:
				returnData = cgbSPRPaletteIndex;
				break;
			case 0xFF6B:
				returnData = cgbSPRPalette[cgbSPRPaletteIndex & 0x3F];
				break;
		}
	}
	return returnData;
}

void GBGPU::updateGPUTimer(int lastCycleCount) {
	int mode = STAT & 0x3;
	GPUCycleCount += lastCycleCount;
	if ((LCDC & 0x80) != 0/*true*/) {	// I honestly don't know if this is conditional
		switch(mode) {
			// HBlank
			case 0:
				if (GPUCycleCount >= 204) {
					GPUCycleCount -= 204;
					//GPUCycleCount = 0;
					LY++;

					// check LY == LYC
					checkLYC();
					
					if (LY == 144) {
						mode = 1;	// Switch to vblank
						if ((STAT & mode1) != 0) {
							interrupts->IF |= (0xE0 | LCDCint);
						}
						// Activate vblank interrupt too
						interrupts->IF |= (0xE0 | vblankint);
						newVblank = true;
					}
					else {
						mode = 2;
						if ((STAT & mode2) != 0) {
							interrupts->IF |= (0xE0 | LCDCint);
						}
					}
				}
				break;
			case 1:
				if (GPUCycleCount >= 456) {
					GPUCycleCount -= 456;
					//GPUCycleCount = 0;
					LY++;

					// check LY == LYC
					checkLYC();

					/*if (LY > 153) {
						mode = 2;
						if ((STAT & mode2) != 0) {
							interrupts->IF |= (0xE0 | LCDCint);
						}
						LY = 0;
						windowLineCounter = 0;
						// check LY == LYC
						checkLYC();
					}*/
					// When we reach line 153, it starts reporting that the line is 0, and activates LYC.
					// However, vblank still lasts for another line. Line 0 essentially gets to last twice as long!
					// Maybe comparable to the pre-redner line of the NES? GB is weird.
					if (LY == 153) {
						LY = 0;
						windowLineCounter = 0;
						checkLYC();
					}
					else if (LY == 1) {
						mode = 2;
						if ((STAT & mode2) != 0) {
							interrupts->IF |= (0xE0 | LCDCint);
						}
						LY = 0;
						// LYC interrupt supposedly doesn't activate once again or anything. Line 0 just needs to be extended.
					}
				}
				break;
			// OAM read
			case 2:
				if (GPUCycleCount >= 80) {
					GPUCycleCount -= 80;
					//GPUCycleCount = 0;
					mode = 3;
				}
				break;
			// VRAM read
			case 3:
				if (GPUCycleCount >= 172) {
					GPUCycleCount -= 172;
					//GPUCycleCount = 0;
					mode = 0;
					if ((STAT & mode0) != 0) {
						interrupts->IF |= (0xE0 | LCDCint);
					}
					// Render scanline before going to hblank
					renderScanline();
					// Activate HDMA if there's HDMA left
					if (HDMAActive && (HDMAlen & 0x7F) >= 0) {
						if ((HDMAlen & 0x7F) == 0) {
							HDMAActive = false;
						}
						HDMA();
						HDMAlen--;
					}
				}
				break;
			default:
				break;
		}	// End of switch case
	}
	else {
		GPUCycleCount = 0;
		LY = 0;
		windowLineCounter = 0;
		mode = 0;	// Set mode to 0 to indicate VRAM is safe
		STAT &= 0xFC;
	}

	// Write current mode to STAT
	STAT &= 0xFC;
	STAT |= mode;
}

void GBGPU::renderScreen(SDL_Window *window, SDL_Renderer *ren) {
	// Gameboy screen: 160x144
	// Necessary surfaces
	//SDL_Surface *mainSurface = SDL_CreateRGBSurface(0, 256, 256, 32, 0, 0, 0, 0);
	SDL_Surface *mainSurface = SDL_CreateRGBSurface(0, 160, 144, 32, 0, 0, 0, 0);
	
	// Copy background surface to the main surface
	SDL_BlitSurface(backgroundGlobal, NULL, mainSurface, NULL);

	// Render main surface
	SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, mainSurface);
	SDL_FreeSurface(mainSurface);
	SDL_RenderClear(ren);
	SDL_RenderCopy(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);

	SDL_DestroyTexture(tex);
}

void GBGPU::drawTile(uint16_t address, SDL_Surface *inSurface, int x, int y, bool background, bool horizontalFlip, bool verticalFlip, uint8_t pallete) {
	uint32_t *surfacePixels = (uint32_t*)inSurface->pixels;
	uint8_t tile[16];

	for (int i = 0; i < 16; i++) {
		tile[i] = VRAM[(address - 0x8000) + i];
	}
	for (int i = 0; i <= 15; i += 2) {
		for (int j = 7; j >= 0; j--) {
			int tempByte = ((tile[i + 1] & 0x1) << 1)|(tile[i] & 0x1);
			int drawX;
			int drawY;
			if (!horizontalFlip) {
				drawX = j + x;
			}
			else {
				drawX = (7 - j) + x;
			}
			if (!verticalFlip) {
				drawY = (i / 2) + y;
			}
			else {
				drawY = (7 - (i / 2)) + y;
			}

			uint32_t color = 0;
			switch (tempByte) {
				case 3:
					// Black
					color = colors[(pallete >> 6) & 0x3];
					//surfacePixels[drawX + (drawY * inSurface->h)] = 0x7CFC00;
					break;
				case 2:
					// Dark Gray
					color = colors[(pallete >> 4) & 0x3];
					break;
				case 1:
					// Light Gray
					color = colors[(pallete >> 2) & 0x3];
					break;
				case 0:
					// white, only do this for background tiles
					if (background) {
						color = colors[(pallete) & 0x3];
						//surfacePixels[drawX + (drawY * inSurface->h)] = 0x636F57;
					}
					else {
						color = 0xFF00EE;
						SDL_SetColorKey(inSurface, true, 0xFF00EE);
					}
					break;
				default:
					break;
			}
			surfacePixels[drawX + (drawY * inSurface->w)] = color;


			tile[i] >>= 1;
			tile[i + 1] >>= 1;
		}
	}
}


void GBGPU::renderScanline() {
	// Declare starting X and Y position
	uint8_t x = SCX;
	uint8_t y = SCY + LY;

	// Run line renderers
	// Background
	if (LCDC & 0x01){
		renderBGLine();
	}
	// Window
	if (LCDC & 0x20){
		renderWindowLine();
	}
	// Sprites
	if (LCDC & 0x02) {
		renderSpriteLine();
	}
	/*else {
		// Fill BG colors
		for (int i = 0; i < 160; i++) {
			lineBuffer[i] = (BGP >> (2 * lineBuffer[i])) & 0x3;
		}
	}*/


	// Declare BG pixels pointer/array
	uint32_t *globalBGPixels = (uint32_t*)backgroundGlobal->pixels;

	for (int i = 0; i < 160; i++) {
		int pixelData = lineBuffer[i];
		if (CGBmode) {
			// BG
			uint8_t* table;
			if ((pixelData & 0x20) == 0x20) {
				table = cgbBGPalette;
			}
			// Sprite
			else {
				table = cgbSPRPalette;
			}
			uint8_t index = (pixelData & 0x1F) << 1;
			pixelData = table[index]|((table[index|1]) << 8);
			pixelData &= 0x7FFF;
			// Translate the BGR555 value to an RGB888
			uint8_t red = (pixelData & 0x1F) << 3;
			red |= red >> 5;
			uint8_t green = ((pixelData >> 5) & 0x1F) << 3;
			green |= green >> 5;
			uint8_t blue = ((pixelData >> 10) & 0x1F) << 3;
			blue |= blue >> 5;
			pixelData = blue | (green << 8) | (red << 16);
			// Write to background surface
			int drawX = i;
			int drawY = LY;
			globalBGPixels[drawX + (drawY * backgroundGlobal->w)] = pixelData;
		}
		else {
			// bgcolor
			if ((pixelData & 0x4) == 0x4) {
				pixelData = (BGP >> (2 * (pixelData & 0x3))) & 0x3;
			}
			// Write to background surface
			int drawX = i;
			int drawY = LY;
			globalBGPixels[drawX + (drawY * backgroundGlobal->w)] = colors[pixelData];
		}
		lineBuffer[i] = 0;
	}
}

void GBGPU::renderBGLine() {
	uint8_t x = SCX;
	uint8_t y = SCY + LY;
	for (int i = 0; i < 160; i++, x++) {
		// Get the current tile number based on this
		int tileNum = (x / 8) + ((y / 8) * 32);

		// Determine the location in memory of the tile
		//uint16_t mapLocation = tileNum + ((LCDC & 0x08) != 0) ? 0x1800 : 0x1C00;
		uint16_t mapLocation = 0x0000;
		uint16_t tileLocation = 0x0000;
		uint8_t mapAttributes = 0x00;	// CGB attributes

		// Get tile location
		if ((LCDC & 0x08) != 0) {
			mapLocation = 0x1C00 + tileNum;
		}
		else {
			mapLocation = 0x1800 + tileNum;
		}
		tileLocation = VRAM[mapLocation];
		if (CGBmode) {
			mapAttributes = VRAM[0x2000 | mapLocation];
		}
		if ((LCDC & 0x10) != 0) {
			tileLocation = (tileLocation << 4);
		}
		else {
			tileLocation = (128 + (int16_t)tileLocation) & 0xff;
			tileLocation = 0x800 + (tileLocation << 4);
		}

		// Use simple modulo to get pixel number of tile
		int pixelX = x % 8;
		int pixelY = y % 8;

		// CGB Attribute parser
		if (CGBmode) {
			if ((mapAttributes & 0x8) == 0x8) {
				tileLocation |= 0x2000;
			}
			if ((mapAttributes & 0x20) == 0x20) {
				pixelX = 7 - pixelX;
			}
			if ((mapAttributes & 0x40) == 0x40) {
				pixelY = 7 - pixelY;
			}
		}

		int pixelData = ((((VRAM[tileLocation + (pixelY * 2) + 1] >> (7 - pixelX))) & 0x1) << 1) |
			((((VRAM[tileLocation + (pixelY * 2)] >> (7 - pixelX))) & 0x1));

		if (CGBmode) {
			lineBuffer[i] = pixelData | ((mapAttributes & 0x7) << 2) | 0x20;
			bgPriorties[i] = (mapAttributes & 0x80) == 0x80;
		}
		else {
			lineBuffer[i] = 0x4 | pixelData;
		}
	}
}

void GBGPU::renderWindowLine() {
	// Check if Window is actually on the line, skip this entirely if it's not
	int windowX = WX - 7;
	int windowY = WY;

	if (windowX >= 160 || windowY > LY) {
		return;
	}

	int x = 7 - WX;
	int y = windowLineCounter;	// I have no idea if I'm implementing this correctly gosh dang
	windowLineCounter++;

	for (int i = 0; i < 160; i++, x++) {
		// Get the current tile number based on this
		if (i >= windowX && x >= 0) {
			int tileNum = (x / 8) + ((y / 8) * 32);

			// Determine the location in memory of the tile
			uint16_t mapLocation = 0x0000;
			uint16_t tileLocation = 0x0000;
			uint8_t mapAttributes = 0x00;	// CGB attributes

			// Get tile location
			if ((LCDC & 0x40) != 0) {
				mapLocation = 0x1C00 + tileNum;
			}
			else {
				mapLocation = 0x1800 + tileNum;
			}
			tileLocation = VRAM[mapLocation];
			if (CGBmode) {
				mapAttributes = VRAM[0x2000 | mapLocation];
			}
			if ((LCDC & 0x10) != 0) {
				tileLocation = (tileLocation << 4);
			}
			else {
				tileLocation = (128 + (int16_t)tileLocation) & 0xff;
				tileLocation = 0x800 + (tileLocation << 4);
			}

			// Use simple modulo to get pixel number of tile
			int pixelX = x % 8;
			int pixelY = y % 8;

			// CGB Attribute parser
			if (CGBmode) {
				if ((mapAttributes & 0x8) == 0x8) {
					tileLocation |= 0x2000;
				}
				if ((mapAttributes & 0x20) == 0x20) {
					pixelX = 7 - pixelX;
				}
				if ((mapAttributes & 0x40) == 0x40) {
					pixelY = 7 - pixelY;
				}
			}

			int pixelData = ((((VRAM[tileLocation + (pixelY * 2) + 1] >> (7 - pixelX))) & 0x1) << 1) |
				((((VRAM[tileLocation + (pixelY * 2)] >> (7 - pixelX))) & 0x1));

			if (CGBmode) {
				lineBuffer[i] = pixelData | ((mapAttributes & 0x7) << 2) | 0x20;
				bgPriorties[i] = (mapAttributes & 0x80) == 0x80;
			}
			else {
				lineBuffer[i] = 0x4 | pixelData;
			}
		}
	}
}

void GBGPU::renderSpriteLine() {
	uint8_t OAM2[40] = {};				// Secondary OAM table
	int spriteCount = 0;			// Keep track of how many sprites are on the line
	int spriteLineBuffer[160] = {};	// Buffer for sprite lines, to be applied over bg
	int spriteSignature[160] = {};	// Stores which sprite is on the pixel cause I dunno how else to handle sprite overlap priority.
	// Initialize signatures and line buffer to all 0
	for (int i = 0; i < 160; i++) {
		spriteLineBuffer[i] = -1;
		spriteSignature[i] = -1;
	}
	bool priorityValues[160] = {};		// BG Priority values per pixel, treating this similar to the NES since I'm not sure of it's real behavior.
	bool tallSprite = (LCDC & 0x4) == 0x4;
	int spriteHeight = tallSprite ? 16 : 8;
	// Check all sprites and see which are on the line, up to 10 sprites
	for (int i = 0; i < 40 && spriteCount < 10; i++) {
		if ((LY >= (OAM[i*4] - 16)) && (LY < (OAM[i * 4] - 16 + spriteHeight))) {
			OAM2[(spriteCount * 4) + 0] = OAM[(i * 4) + 0];
			OAM2[(spriteCount * 4) + 1] = OAM[(i * 4) + 1];
			OAM2[(spriteCount * 4) + 2] = OAM[(i * 4) + 2];
			OAM2[(spriteCount * 4) + 3] = OAM[(i * 4) + 3];
			spriteCount++;
		}
	}

	if (spriteCount == 0) {
		// Fill BG colors
		/*
		for (int i = 0; i < 160; i++) {
			lineBuffer[i] = (BGP >> (2 * lineBuffer[i])) & 0x3;
		}*/
		return;
	}

	for (int i = 0; i < spriteCount; i++) {
		uint8_t spriteY = OAM2[(i * 4) + 0] - 16;
		uint8_t spriteX = OAM2[(i * 4) + 1] - 8;
		uint8_t spriteTile = OAM2[(i * 4) + 2];
		uint8_t spriteAttributes = OAM2[(i * 4) + 3];

		// Set pixelY, accounting for vertical flip
		uint8_t pixelY = (spriteAttributes & 0x40) ? (spriteHeight - 1) - (LY - spriteY) : LY - spriteY;

		// For tall sprites, re-adjust spriteTile pointer accordingly
		if (tallSprite) {
			if (pixelY < 8) {
				spriteTile &= 0xFE;
			}
			else {
				spriteTile |= 0x1;
			}
			pixelY = pixelY & 0x7;	// mask the value if it's tall
		}
		uint16_t tilePointer = spriteTile << 4;
		if (CGBmode && (spriteAttributes & 0x8) == 0x8) {
			tilePointer |= 0x2000;
		}
		uint8_t tileByte0 = VRAM[tilePointer + (2*pixelY)];
		uint8_t tileByte1 = VRAM[tilePointer + (2 * pixelY) + 1];

		// Check if a sprite is already here, mark if that sprite should have priority over this one
		// Sprites with the same X value are hidden underneath at lower priorities, this should handle that case.
		int higherPrioritySprite = i;
		
		if (spriteSignature[spriteX] >= 0) {
			higherPrioritySprite = spriteSignature[spriteX];
		}

		for (int j = 0; j < 8; j++) {
			// Don't draw off screen, break if we're about to (don't want out of bounds array access).
			/*if ((spriteX + j) >= 160){
				break;
			}*/
			// Assign the pixel X value, then get the pixels value (0-3)
			int pixelX = ((spriteAttributes & 0x20) ? (7 - j) : j);
			uint32_t pixelNumber = ((tileByte0 >> (7 - pixelX)) & 0x1) | (((tileByte1 >> (7 - pixelX)) & 0x1) << 1);
			//lineBuffer[i] = (BGP >> (2*pixelData)) & 0x3;
			// If we can write a pixel here, then write a pixel here
			uint8_t pixelNum = spriteX + j;
			// There's probably another way of dealing with priority values ie checking a given sprite on the line's X Value against the currents. Might fix some edge-cases.
			// In CGB mode, sprite overlap priority always goes by OAM ordering. IE: 0 sprites are always on top.
			if ((((spriteSignature[pixelNum] != higherPrioritySprite) && !CGBmode) || spriteLineBuffer[pixelNum] == -1) && (pixelNum) < 160) {
				if (pixelNumber != 0) {
					if (CGBmode) {
						spriteLineBuffer[pixelNum] = pixelNumber | ((spriteAttributes & 0x7) << 2);
					}
					else {
						spriteLineBuffer[pixelNum] = (((spriteAttributes & 0x10) ? OBP1 : OBP0) >> (2 * pixelNumber)) & 0x3;
					}
				}
				// Sprite Signature field is stricter, won't be overwritten just because of a transparent pixel
				if (spriteSignature[pixelNum] != higherPrioritySprite) {
					spriteSignature[pixelNum] = i;
				}
				priorityValues[pixelNum] = (spriteAttributes & 0x80) == 0x80;	// True if it's behind
			}
		}
	}
	// Line buffer mixing
	for (int i = 0; i < 160; i++) {
		// Either the sprite is always in front, or there's a 0 pixel there (meaning 
		if (((!priorityValues[i] && !bgPriorties[i]) || (lineBuffer[i] & 0x3) == 0) && spriteLineBuffer[i] != -1) {
			lineBuffer[i] = spriteLineBuffer[i];
		}
		/*else {
			// Fill BG colors
			lineBuffer[i] = (BGP >> (2 * lineBuffer[i])) & 0x3;
		}*/
	}
}



void GBGPU::checkLYC() {
	// Interrupt on lyc = ly coincidence
	if (LY == LYC) {
		STAT |= 0x4;
	}
	else {
		STAT &= 0xFB;
	}
	if ((STAT & coincidence) != 0 && (STAT & 0x04) != 0) {
		interrupts->IF |= (0xE0 | LCDCint);
	}
}

void GBGPU::HDMA()
{
	// CGB HDMA transfer
	for (int j = 0; j < 0x10; j++) {
		// I'm assuming this is affected by VRAM bank?
		if (HDMAsrc < 0xC000) {
			sendData(HDMAdst, gbCart->recieveData(HDMAsrc));
		}
		else {
			sendData(HDMAdst, wram->recieveData(HDMAsrc));
		}
		HDMAdst++;
		HDMAsrc++;
	}
}
