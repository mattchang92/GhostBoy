#include "WRAM.h"



WRAM::WRAM()
{
}


WRAM::~WRAM()
{
}

void WRAM::sendData(uint16_t address, uint8_t data)
{
	if (address >= 0xC000 && address <= 0xFDFF) {
		uint16_t wramAddress = (address & 0x1FFF);
		if (wramAddress >= 0x1000) {
			wramAddress &= 0xFFF;
			wramAddress |= WRAMBank << 12;
		}
		RAM[wramAddress] = data;	// Mask covers echo area too
	}
	else if (address == 0xFF70) {
		WRAMBank = data & 0x7;
		if (WRAMBank == 0) {
			WRAMBank = 1;
		}
	}
}

uint8_t WRAM::recieveData(uint16_t address)
{
	uint8_t returnVal = 0xFF;
	if (address >= 0xC000 && address <= 0xFDFF) {
		uint16_t wramAddress = (address & 0x1FFF);
		if (wramAddress >= 0x1000) {
			wramAddress &= 0xFFF;
			wramAddress |= WRAMBank << 12;
		}
		returnVal = RAM[wramAddress];	// Mask covers echo area too
	}
	else if (address == 0xFF70) {
		returnVal = WRAMBank;
	}
	return returnVal;
}
