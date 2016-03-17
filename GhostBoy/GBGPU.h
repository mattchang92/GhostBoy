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
	uint8_t recieveData(uint16_t address);
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
	// Cycle Counter
	int GPUCycleCount = 0;
	// GPU Held memory
	uint8_t VRAM[0x9FFF - 0x800 + 1] = {};
	uint8_t OAM[0xFe9F - 0xFE00 + 1] = {};
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
};

