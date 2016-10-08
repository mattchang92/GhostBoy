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
	int screenMultiplier = 2;
	// Launch arguments
	bool launchError = false;
	if(argc > 1){
		romFilePath = argv[1];
	}
	else {
		launchError = true;
	}
	if (argc > 2) {
		try {
			screenMultiplier = stoi(argv[2]);
		}
		catch (invalid_argument e) {
			launchError = true;
		}
	}
	if (launchError) {
		cout << "Usage: GhostBoy romfile screenmultiplier\n";
		exit(-1);
	}

	// Creat obbjects
	Cartridge* gbCart = Cartridge::getCartridge(romFilePath);
	Interrupts interrupts;
	Timer timer(interrupts);
	GBGPU gbgpu(interrupts);
	Input input;
	APU apu;
	Memory mainMem (gbCart, interrupts, timer, gbgpu, input, apu);
	GBCPU CPU (mainMem);
	// SDL Stuff
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = 0;
	window = SDL_CreateWindow("GhostBoy SDL Window", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		160 * screenMultiplier, 144 * screenMultiplier,
		SDL_WINDOW_SHOWN);
	SDL_Renderer *ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Event events;

	int cycleTotal = 0;
	int vblankCount = 0;
	

	// Check if bootstrap file is present. If it is, load it in, if not, skip the bootstrap.
	if (mainMem.setBootstrap(ifstream("boot.rom", ios::in | ios::binary | ios::ate))) {
		CPU.resetGBBios();
	}
	else {
		CPU.resetGBNoBios();
	}

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
		while (!gbgpu.newVblank) {
			CPU.executeOneInstruction();
			timer.updateTimers(CPU.getLastCycleCount());
			gbgpu.updateGPUTimer(CPU.getLastCycleCount());
			apu.step(CPU.getLastCycleCount());
			
			//cycleTotal += CPU.getLastCycleCount();
		}
		//apu.playSound();	// Play buffered sound?
		//cout << "Total cycles this frame: " << cycleTotal << "\n";
		//cycleTotal = 0;
		// Stuff to run after a vblank occurs
		vblankCount++;
		//cout << "Vblank count: " << vblankCount << "\n";
		gbgpu.newVblank = false;
		if ((gbgpu.recieveData(0xFF40) & 0x80) != 0/*true*/) {
			gbgpu.renderScreen(window, ren);
		}
	}

	// Run gbCart save for fallback
	gbCart->saveBatteryData();
	
    return 0;
}

