#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <fstream>

using namespace std;

#pragma once
class Cartridge
{
public:
	// Constructor
	Cartridge(string romPath, bool bootStrap);
	~Cartridge();
	// Public functions
	void sendData(uint16_t address, uint8_t input);
	uint8_t recieveData(uint16_t address);


private:
	// Private functions

	// Private variables and classes
	bool bootStrap;
	uint8_t extRam[4][0xBFFF - 0xA000 + 1];	// Somewhat temporary
	ifstream romFileStream;
	// MBC1 registers
	bool ramEnable;
	int romBankNumber;
	int ramBankNumber;
	bool romRamMode;

	// Dynamically allocated data pointers
	uint8_t *romData;

};

