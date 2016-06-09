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

using namespace std;

int main(int argc, char* argv[])
{
	
	//Cartridge gbCart("roms/cpu_instrs/individual/09-op r,r.gb", false);
	//Cartridge gbCart("roms/cpu_instrs/cpu_instrs.gb", false);
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
		cout << "Usage: GhostBoy.exe romfile screenmultiplier\n";
		exit(-1);
	}

	// Creat obbjects
	Cartridge gbCart(romFilePath, false);
	Interrupts interrupts;
	Timer timer(interrupts);
	GBGPU gbgpu(interrupts);
	Input input;
	Memory mainMem (gbCart, interrupts, timer, gbgpu, input);
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

	

	CPU.resetGBNoBios();

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

			//TEMP
			//mainMem.writeByte(0xFF00, 0xFF);
		}
		// Stuff to run after a vblank occurs
		gbgpu.newVblank = false;
		if ((gbgpu.recieveData(0xFF40) & 0x80) != 0) {
			gbgpu.renderScreen(window, ren);
		}
	}
	
    return 0;
}

