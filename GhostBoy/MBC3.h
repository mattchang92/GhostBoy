#pragma once
#include "Cartridge.h"

class MBC3 :
	public Cartridge
{
public:
	MBC3(uint8_t* romData, unsigned int romSize, unsigned int ramSize);
	~MBC3();
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

	//uint8_t extRAM[4][0xBFFF - 0xA000 + 1];	// Somewhat temporary
	uint8_t* extRAM;
	bool ramEnable = false;
	int romBankNumber = 1;
	uint8_t RAMRTCselect = 0;
};