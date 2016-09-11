#pragma once
#include "Cartridge.h"

class MBC5 :
	public Cartridge
{
public:
	MBC5(uint8_t* romData, int romSize);
	~MBC5();

	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);
private:
	uint8_t* romData;
	unsigned int romSize;

	bool ramEnable = false;
	uint16_t romBankNumber = 0;
	uint8_t ramBankNumber = 0;

	uint8_t extRAM[16][0xBFFF - 0xA000 + 1];
};

