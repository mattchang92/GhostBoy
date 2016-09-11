#include <stdint.h>
#include "Cartridge.h"
#include "Interrupts.h"
#include "Timer.h"
#include "GBGPU.h"
#include "Input.h"

#pragma once
class Memory
{
public:
	Memory(Cartridge* gbCart, Interrupts &interrupts, Timer &timer, GBGPU &gbgpu, Input &input);
	~Memory();
	uint8_t readByte(uint16_t address);
	uint16_t readWord(uint16_t address);
	void writeByte(uint16_t address, uint8_t data);
	void writeWord(uint16_t address, uint16_t data);
	void writeByteNoProtect(uint16_t address, uint8_t data);

private:
	// Class pointers
	Cartridge *gbCart;
	Interrupts *interrupts;
	Timer *timer;
	GBGPU *gbgpu;
	Input *input;
	// Internal arrays and variables
	//uint8_t tempMem[0xFFFF + 1] = {};	// Temp storage map
	uint8_t RAM[0x2000] = {0};			// 8KB RAM 
	uint8_t highRAM[0x7F] = {0};			// 127 Bytes high-ram
};