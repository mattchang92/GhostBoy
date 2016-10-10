#pragma once
#include "Cartridge.h"

class MBC5 :
	public Cartridge
{
public:
	MBC5(uint8_t* romData, unsigned int romSize, unsigned int ramSize);
	~MBC5();

	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);

	// Battery functions
	void setBatteryLocation(string batteryPath);
	void saveBatteryData();

private:
	uint8_t* romData;
	unsigned int romSize;
	unsigned int ramSize;
	bool battery = false;
	string batteryPath = "";
	bool ramNewData = false;

	bool ramEnable = false;
	uint16_t romBankNumber = 0;
	uint8_t ramBankNumber = 0;

	uint8_t* extRAM;
};

