#include <stdint.h>
#include "Cartridge.h"
#include "Interrupts.h"
#include "Timer.h"
#include "GBGPU.h"
#include "Input.h"
#include "APU.h"
#include "WRAM.h"
#include "LinkCable.h"

#pragma once
class Memory
{
public:
	Memory(Cartridge* gbCart, Interrupts &interrupts, Timer &timer, GBGPU &gbgpu, Input &input, APU &apu, WRAM &wram, bool CGBMode, LinkCable &linkCable);
	~Memory();
	uint8_t readByte(uint16_t address);
	uint16_t readWord(uint16_t address);
	void writeByte(uint16_t address, uint8_t data);
	void writeWord(uint16_t address, uint16_t data);
	void writeByteNoProtect(uint16_t address, uint8_t data);
	bool setBootstrap(ifstream bootstrapstream);

private:
	bool CGBMode = false;
	// Class pointers
	Cartridge *gbCart;
	Interrupts *interrupts;
	Timer *timer;
	GBGPU *gbgpu;
	Input *input;
	APU *apu;
	WRAM *wram;
	LinkCable *linkCable;
	// Internal arrays and variables
	//uint8_t tempMem[0xFFFF + 1] = {};	// Temp storage map
	//uint8_t RAM[0x8000] = {0};			// 8KB RAM 
	uint8_t highRAM[0x7F] = {0};			// 127 Bytes high-ram
	bool bootStrapActive = false;
	uint8_t bootstrap[0x100];
	uint8_t serialByte;
	// CGB Stuff
	uint8_t WRAMBank = 1;	// CGB Double Speed mode switch
	uint8_t key1 = 0;		// WRAM Bank for CGB bank switch
};