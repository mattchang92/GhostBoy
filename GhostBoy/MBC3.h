#pragma once
#include "Cartridge.h"

class MBC3 :
	public Cartridge
{
public:
	MBC3(uint8_t* romData, int romSize);
	~MBC3();
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);

private:
	uint8_t* romData;
	unsigned int romSize;

	uint8_t extRAM[4][0xBFFF - 0xA000 + 1];	// Somewhat temporary
	bool ramEnable = false;
	int romBankNumber = 1;
	uint8_t RAMRTCselect = 0;
};