#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <fstream>

using namespace std;

#pragma once
class Cartridge
{
public:
	Cartridge();
	~Cartridge();

	virtual void sendData(uint16_t address, uint8_t data) = 0;
	virtual uint8_t recieveData(uint16_t address) = 0;

	// Save battery definers
	virtual void setBatteryLocation(string batteryPath) = 0;
	virtual void saveBatteryData() = 0;

	static Cartridge* getCartridge(string romPath);
	static bool loadBatteryFile(uint8_t* extRAm, unsigned int ramSize, string inBatteryPath);
	static void saveBatteryFile(uint8_t* extRAM, unsigned int ramSize, string inBatteryPath);
};

