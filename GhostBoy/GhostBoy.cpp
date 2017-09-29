// GhostBoy.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include<stdio.h>
#include <stdlib.h>
#include <string>
#include "SDL.h"
#include "Memory.h"
#include "GBCPU.h"
#include "stdint.h"
#include "Interrupts.h"
#include "GBGPU.h"
#include "APU.h"

using namespace std;

int main(int argc, char* argv[])
{
	// Handle arguments
	string romFilePath = "";
	string romFilePath2 = "";
	int screenMultiplier = 2;
	// Launch arguments
	bool launchError = false;
	if(argc > 2){
		romFilePath = argv[1];
		romFilePath2 = argv[2];
	}
	else {
		launchError = true;
	}
	if (argc > 3) {
		try {
			screenMultiplier = stoi(argv[3]);
		}
		catch (invalid_argument e) {
			launchError = true;
		}
	}
	if (launchError) {
		cout << "Usage: " << argv[0] << " romfile screenmultiplier\n";
		exit(-1);
	}

	// SDL Stuff
	int foo = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (foo == 0) {
		cout << SDL_GetError() << "\n";
	}
	SDL_Window *window = 0;
	window = SDL_CreateWindow("GhostBoy SDL Window",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(160 * screenMultiplier) * 2 + 1, 144 * screenMultiplier,
		SDL_WINDOW_SHOWN);
	SDL_Renderer *ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Event events;


	// Creat obbjects
	Cartridge* gbCart = Cartridge::getCartridge(romFilePath);
	Interrupts interrupts;
	Timer timer(interrupts);
	WRAM wram;
	bool CGBMode = (gbCart->recieveData(0x143) & 0x80) == 0x80;
	GBGPU gbgpu(interrupts, gbCart, wram, CGBMode);
	Input input(false);
	APU apu;
	LinkCable linkCable(interrupts);
	Memory mainMem (gbCart, interrupts, timer, gbgpu, input, apu, wram, CGBMode, linkCable);
	GBCPU CPU (mainMem);

	int cycleTotal = 0;
	int vblankCount = 0;
	

	// Check if bootstrap file is present. If it is, load it in, if not, skip the bootstrap.
	
	if (!CGBMode) {
		if (mainMem.setBootstrap(ifstream("boot.rom", ios::in | ios::binary | ios::ate))) {
			CPU.resetGBBios();
		}
		else {
			CPU.resetGBNoBios();
		}
	}
	else {
		CPU.resetCGBNoBios();
	}

	// COPY OF THE CODE ABOVE FOR A SECOND GAMEBOY
	// Creat obbjects
	Cartridge* gbCart2 = Cartridge::getCartridge(romFilePath2);
	Interrupts interrupts2;
	Timer timer2(interrupts2);
	WRAM wram2;
	bool CGBMode2 = (gbCart2->recieveData(0x143) & 0x80) == 0x80;
	GBGPU gbgpu2(interrupts2, gbCart2, wram2, CGBMode2);
	Input input2(true);
	APU apu2;
	LinkCable linkCable2(interrupts2);
	Memory mainMem2(gbCart2, interrupts2, timer2, gbgpu2, input2, apu2, wram2, CGBMode2, linkCable2);
	GBCPU CPU2(mainMem2);

	int cycleTotal2 = 0;
	int vblankCount2 = 0;


	// Check if bootstrap file is present. If it is, load it in, if not, skip the bootstrap.

	if (!CGBMode2) {
		if (mainMem2.setBootstrap(ifstream("boot.rom", ios::in | ios::binary | ios::ate))) {
			CPU2.resetGBBios();
		}
		else {
			CPU2.resetGBNoBios();
		}
	}
	else {
		CPU2.resetCGBNoBios();
	}
	// END COPY


	// Main loop
	bool running = true;
	while (running) {
		//SDL Events check
		SDL_PumpEvents();
		while (SDL_PollEvent(&events)) {
			if (events.type == SDL_QUIT) {
				running = false;
			}
		}
		// Main CPU loop
		while (!gbgpu.newVblank || !gbgpu2.newVblank) {
			if (!gbgpu.newVblank) {
				CPU.executeOneInstruction();
				int lastCycleCount = CPU.getLastCycleCount();
				if (CPU.getDoubleSpeed()) {
					gbgpu.updateGPUTimer(lastCycleCount / 2);
					apu.step(lastCycleCount / 2);
				}
				else {
					gbgpu.updateGPUTimer(lastCycleCount);
					apu.step(lastCycleCount);
				}
				timer.updateTimers(lastCycleCount);
				//cycleTotal += CPU.getLastCycleCount();
			}
			// GB2
			if (!gbgpu2.newVblank) {
				CPU2.executeOneInstruction();
				int lastCycleCount2 = CPU2.getLastCycleCount();
				if (CPU2.getDoubleSpeed()) {
					gbgpu2.updateGPUTimer(lastCycleCount2 / 2);
					apu2.step(lastCycleCount2 / 2);
				}
				else {
					gbgpu2.updateGPUTimer(lastCycleCount2);
					apu2.step(lastCycleCount2);
				}
				timer2.updateTimers(lastCycleCount2);
				//cycleTotal += CPU.getLastCycleCount();
			}
			// Link cable check
			
		}
		if ((linkCable.getTransferRequest() && linkCable.getMaster()) || (linkCable2.getTransferRequest() && linkCable2.getMaster())) {
			uint8_t swapSB = linkCable.getSBout();
			linkCable.setSBin(linkCable2.getSBout());
			linkCable2.setSBin(swapSB);
			linkCable.transferComplete();
			linkCable2.transferComplete();
		}


		//apu.playSound();	// Play buffered sound?
		//cout << "Total cycles this frame: " << cycleTotal << "\n";
		//cycleTotal = 0;
		// Stuff to run after a vblank occurs
		vblankCount++;
		vblankCount2++;
		//cout << "Vblank count: " << vblankCount << "\n";
		gbgpu.newVblank = false;
		gbgpu2.newVblank = false;
		// Clear and Present render outside of the GPU clas
		// Otherwise, you get flickering
		SDL_RenderClear(ren);
		gbgpu.renderScreen(window, ren, 0, 0, 160 * screenMultiplier, 144 * screenMultiplier);
		gbgpu2.renderScreen(window, ren, (160 * screenMultiplier)+1, 0, 160 * screenMultiplier, 144 * screenMultiplier);
		SDL_RenderPresent(ren);
	}

	// Run gbCart save for fallback
	gbCart->saveBatteryData();
	
    return 0;
}

