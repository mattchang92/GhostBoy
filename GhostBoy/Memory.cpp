#include "Memory.h"
#include "stdio.h"



Memory::Memory(Cartridge* gbCart, Interrupts &interrupts, Timer &timer, GBGPU &gbgpu, Input &input) : gbCart(gbCart), 
interrupts(&interrupts), timer(&timer), gbgpu(&gbgpu), input(&input)
{
}


Memory::~Memory()
{
}

uint8_t Memory::readByte(uint16_t address)
{
	uint8_t returnVal = 0xFF;
	// Cartridge space
	if (address >= 0x0000 && address <= 0x7FFF) {
		returnVal = gbCart->recieveData(address);
	}
	// VRAM
	else if (address >= 0x8000 && address <= 0x9FFF) {
		returnVal = gbgpu->recieveData(address);
	}
	// Cartridge RAM space
	else if (address >= 0xA000 && address <= 0xBFFF) {
		returnVal = gbCart->recieveData(address);
	}
	// RAM
	else if (address >= 0xC000 && address <= 0xFDFF) {
		returnVal = RAM[address & 0x1FFF];	// Mask covers echo area too
	}
	// Controller
	else if (address == 0xFF00) {
		returnVal = input->recieveData();
	}
	// OAM
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		returnVal = gbgpu->recieveData(address);
	}
	// Timer registers
	else if (address >= 0xFF04 && address <= 0xFF07) {
		returnVal = timer->recieveData(address);
	}
	// IF
	else if (address == 0xFF0F) {
		returnVal = interrupts->IF;
	}
	// Sound registers
	else if (address >= 0xFF10 && address <= 0xFF3F) {
		// TODO: Sound stuff. Return 00 for now (some games lock checking sound state if we return 0xFF).
		returnVal = 0x00;
	}
	// GPU registers
	else if (address >= 0xFF40 && address <= 0xFF4B) {
		returnVal = gbgpu->recieveData(address);
	}
	// High RAM area
	else if (address >= 0xFF80 && address <= 0xFFFE) {
		returnVal = highRAM[address & 0x7f];
	}
	// IE
	else if (address == 0xFFFF) {
		returnVal = interrupts->IE;
	}
	return returnVal;
}

uint16_t Memory::readWord(uint16_t address)
{
	uint16_t returnValue;
	returnValue = readByte(address);
	returnValue |= readByte(address + 1) << 8;
	return returnValue;
}

void Memory::writeByte(uint16_t address, uint8_t data)
{
	// Cartridge space
	if (address >= 0x0000 && address <= 0x7FFF) {
		gbCart->sendData(address, data);
	}
	// VRAM
	else if (address >= 0x8000 && address <= 0x9FFF) {
		gbgpu->sendData(address, data);
	}
	// Cartridge RAM space
	else if (address >= 0xA000 && address <= 0xBFFF) {
		gbCart->sendData(address, data);
	}
	// RAM
	else if (address >= 0xC000 && address <= 0xFDFF) {
		RAM[address & 0x1FFF] = data;	// Mask covers echo area too
	}
	// Controller
	else if (address == 0xFF00) {
		input->pollControl(data);
	}
	// OAM
	else if (address >= 0xFE00 && address <= 0xFE9F) {
		gbgpu->sendData(address, data);
	}
	// Timer registers
	else if (address >= 0xFF04 && address <= 0xFF07) {
		timer->sendData(address, data);
	}
	// IF interrupt flag
	else if (address == 0xFF0F) {
		interrupts->IF = data;
	}
	// GPU registers
	else if (address >= 0xFF40 && address <= 0xFF4B) {
		// Inaccurate DMA transfer
		if (address == 0xFF46) {
			uint16_t OAMReadAddress = data << 8;
			for (int i = 0; i <= 0x9F; i++) {
				gbgpu->sendData(0xFE00 + i, readByte(OAMReadAddress + i));
			}
		}
		// Regular access
		else {
			gbgpu->sendData(address, data);
		}
	}
	// High RAM area
	else if (address >= 0xFF80 && address <= 0xFFFE) {
		highRAM[address & 0x7f] = data;
	}
	// IE interrupt flag
	else if (address == 0xFFFF) {
		interrupts->IE = data;
	}
	else{
		if (address == 0xFF02 && data == 0x81) {
			cout << readByte(0xFF01);
		}
	}
}

void Memory::writeWord(uint16_t address, uint16_t data)
{
	writeByte(address, data & 0xff);
	writeByte(address + 1, (data >> 8) & 0xff);
}

void Memory::writeByteNoProtect(uint16_t address, uint8_t data)
{
}
