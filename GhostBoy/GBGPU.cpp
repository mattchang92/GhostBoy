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


GBGPU::GBGPU(Interrupts &interrupts) : interrupts(&interrupts)
{
}


GBGPU::~GBGPU()
{
}

void GBGPU::sendData(uint16_t address, uint8_t data) {
	// TODO: these can't always be written to
	if (address >= 0x8000 && address <= 0x9FFF) {
		VRAM[address - 0x8000] = data;
	}
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		OAM[address - 0xFE00] = data;
	}
	// Register write
	else if (address >= 0xFF40 && address <= 0xFF4B) {
		switch (address) {
			case LCDCbyte:
				LCDC = data;
				break;
			case STATbyte:
				STAT = data;
				break;
			case SCYbyte:
				SCY = data;
				break;
			case SCXbyte:
				SCX = data;
				break;
			case LYbyte:
				LY = 0;
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
}

uint8_t GBGPU::recieveData(uint16_t address) {
	uint8_t returnData = 0;

	if (address >= 0x8000 && address <= 0x9FFF) {
		returnData = VRAM[address - 0x8000];
	}
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		returnData = OAM[address - 0xFE00];
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
	return returnData;
}

void GBGPU::updateGPUTimer(int lastCycleCount) {
	int mode = STAT & 0x3;
	GPUCycleCount += lastCycleCount;
	if (/*(LCDC & 0x80) != 0*/true) {	// I honestly don't know if this is conditional
		switch(mode) {
			// HBlank
			case 0:
				if (GPUCycleCount >= 204) {
					GPUCycleCount -= 204;
					//GPUCycleCount = 0;
					LY++;
					
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

					if (LY > 153) {
						mode = 2;
						if ((STAT & mode2) != 0) {
							interrupts->IF |= (0xE0 | LCDCint);
						}
						LY = 0;
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
				}
				break;
			default:
				break;
		}	// End of switch case
	}
	else {
		GPUCycleCount = 0;
	}
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

	// Window
	if ((LCDC & 0x20) != 0) {
		SDL_Surface *window = SDL_CreateRGBSurface(0, 256, 256, 32, 0, 0, 0, 0);
		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				uint16_t tileLocation = 0;
				// Map could be 0x9800 or 0x9C00
				if ((LCDC & 0x40) != 0) {
					tileLocation = recieveData(0x9C00 + (i * 32) + j);
				}
				else {
					tileLocation = recieveData(0x9800 + (i * 32) + j);
				}
				if ((LCDC & 0x10) != 0) {
					drawTile(0x8000 + (tileLocation << 4), window, j * 8, i * 8, true, false, false, BGP);
				}
				else {
					tileLocation = (128 + (int16_t)tileLocation) & 0xff;
					drawTile(0x8800 + (tileLocation << 4), window, j * 8, i * 8, true, false, false, BGP);
				}
			}
		}
		SDL_Rect windowRect;
		windowRect.w = 256;
		windowRect.h = 256;

		windowRect.x = WX - 7;
		windowRect.y = WY;
		SDL_BlitSurface(window, NULL, mainSurface, &windowRect);
		
		SDL_FreeSurface(window);
	}
	// Sprites
	// Render sprites from OAM, copy to main surface
	bool spriteSize = (LCDC & 0x4) != 0;	// Determine if 8x8 or 8x16, false true
	for (int i = 0; i < 40; i++) {
		if((OAM[i*4] > 0 && OAM[i*4] < 160)
			&& (OAM[(i * 4) + 1] > 0 && OAM[(i * 4) + 1] < 168)) {
			SDL_Surface *sprite;
			bool xFlip = (OAM[(i * 4) + 3] & 0x20) != 0;
			bool yFlip = (OAM[(i * 4) + 3] & 0x40) != 0;
			// OBPnum contains correct pallete bank
			uint8_t OBPnum = (OAM[(i * 4) + 3] & 0x10) != 0 ? OBP1 : OBP0;

			if (!spriteSize) {
				sprite = SDL_CreateRGBSurface(0, 8, 8, 32, 0, 0, 0, 0);
				drawTile(0x8000 + (OAM[(i * 4) + 2] << 4), sprite, 0, 0, 
					false, xFlip, yFlip, OBPnum);
			}
			else {
				// If y is flipped, 8x16 sprite has to be rendered upside down
				sprite = SDL_CreateRGBSurface(0, 8, 16, 32, 0, 0, 0, 0);
				drawTile(0x8000 + ((OAM[(i * 4) + 2] & 0xFE) << 4), sprite, 0, yFlip ? 8 : 0,
					false, xFlip, yFlip, OBPnum);
				drawTile(0x8000 + ((OAM[(i * 4) + 2] | 0x01) << 4), sprite, 0, yFlip ? 0 : 8,
					false, xFlip, yFlip, OBPnum);
			}
			// TODO: Flip it or whatever
			// Put it on main surface
			SDL_Rect spriteRect;
			spriteRect.x = (OAM[(i*4)+1] - 8);
			spriteRect.y = (OAM[(i*4)] - 16);
			SDL_BlitSurface(sprite, NULL, mainSurface, &spriteRect);
			SDL_FreeSurface(sprite);
		}
	}
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
		tile[i] = recieveData(address + i);
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

	// Declare BG pixels pointer/array
	uint32_t *globalBGPixels = (uint32_t*)backgroundGlobal->pixels;

	for (int i = 0; i < 160; i++, x++) {
		// Get the current tile number based on this
		int tileNum = (x / 8) + ((y / 8) * 32);

		// Determine the location in memory of the tile
		uint16_t tileLocation = 0x0000;

		if ((LCDC & 0x08) != 0) {
			tileLocation = recieveData(0x9C00 + tileNum);
		}
		else {
			tileLocation = recieveData(0x9800 + tileNum);
		}
		if ((LCDC & 0x10) != 0) {
			tileLocation = 0x8000 + (tileLocation << 4);
		}
		else {
			tileLocation = (128 + (int16_t)tileLocation) & 0xff;
			tileLocation = 0x8800 + (tileLocation << 4);
		}

		// Use simple modulo to get pixel number of tile
		int pixelX = x % 8;
		int pixelY = y % 8;

		int pixelData = ((((recieveData(tileLocation + (pixelY * 2) + 1) >> (7 - pixelX))) & 0x1) << 1)|
			((((recieveData(tileLocation + (pixelY * 2)) >> (7 - pixelX))) & 0x1));

		uint32_t color = 0;
		switch (pixelData) {
			case 3:
				// Black
				color = colors[(BGP >> 6) & 0x3];
				break;
			case 2:
				// Dark Gray
				color = colors[(BGP >> 4) & 0x3];
				break;
			case 1:
				// Light Gray
				color = colors[(BGP >> 2) & 0x3];
				break;
			case 0:
				// white, only do this for background tiles
				color = colors[(BGP)& 0x3];
				//surfacePixels[drawX + (drawY * inSurface->h)] = 0x636F57;
				break;
			default:
				break;
		}
		// Write to background surface
		int drawX = i;
		int drawY = LY;
		globalBGPixels[drawX + (drawY * backgroundGlobal->w)] = color;
	}
}