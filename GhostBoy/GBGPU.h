#include "Interrupts.h"
#include "stdint.h"
#include "SDL.h"

#pragma once
class GBGPU
{
public:
	GBGPU(Interrupts &interrupts);
	~GBGPU();
	// Data transfer
	void sendData(uint16_t address, uint8_t data);
	uint8_t receiveData(uint16_t address);
	// Updates GPU timing
	void updateGPUTimer(int lastCycleCount);
	// Called to render the screen
	void renderScreen(SDL_Window *window, SDL_Renderer *ren);
	// Keeps track of a new vblank
	// Useful for framerate control
	bool newVblank = false;


private:
	// Interrupts pointer
	Interrupts *interrupts;
	// Private drawing methods
	// Draw Tile: Address of the tile, surface to draw tile to, X and Y coordinates of where to draw to on surface
	void drawTile(uint16_t address, SDL_Surface *inSurface, int x, int y, bool background, bool horizontalFlip, bool verticalFlip, uint8_t pallete);
	// Renders one scanline of the background (at least for now, not sure about sprites)
	// Global backgroud image
	SDL_Surface *backgroundGlobal = SDL_CreateRGBSurface(0, 160, 144, 32, 0, 0, 0, 0);
	void renderScanline();
	// Pixel data holders
	uint32_t lineBuffer[160] = {};
	//uint32_t windowLine[160];
	//uint32_t spriteLine[160];
	//bool spritePriorities[160];
	void renderBGLine();
	void renderWindowLine();
	uint8_t windowLineCounter;
	void renderSpriteLine();
	// LYC Check
	void checkLYC();
	// Cycle Counter
	int GPUCycleCount = 0;
	// OAM Table struct
	struct OAMEntry {
		uint8_t y, x, tileNum, attributes;
	};

	// GPU Held memory
	uint8_t VRAM[0x9FFF - 0x800 + 1] = {};
	uint8_t OAM[0xFe9F - 0xFE00 + 1] = {};
	//OAMEntry OAMTable[0xFe9F - 0xFE00 + 1] = {};
	// GPU Registers
	uint8_t LCDC = 0;
	uint8_t STAT = 0;
	uint8_t SCY = 0;
	uint8_t SCX = 0;
	uint8_t LY = 0;
	uint8_t LYC = 0;
	uint8_t BGP = 0;
	uint8_t OBP0 = 0;
	uint8_t OBP1 = 0;
	uint8_t WY = 0;
	uint8_t WX = 0;


	// Pallete color
	// Black and white pallete
	//uint32_t colors[4] = {0xFFFFFF, 0xD3D3D3, 0xA9A9A9, 0x000000};
	// Greenish pallete (Wiki said so)
	//uint32_t colors[4] = {0x9BBC0F, 0x8BAC0F, 0x306230, 0x0F380F};
	// Whatever BGB used
	uint32_t colors[4] = { 0xE0F8D0, 0x88C070, 0x346856, 0x081820 };
	// Gimmicky Purple
	//uint32_t colors[4] = {0x946DFF, 0x7442FF, 0x3F00FF, 0x170063};
};

