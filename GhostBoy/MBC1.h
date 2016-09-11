#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <fstream>

#include "Cartridge.h"

using namespace std;

#pragma once
class MBC1 :
	public Cartridge
{
public:
	// Constructor
	MBC1(uint8_t* romData, int romSize);
	~MBC1();
	// Public functions
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);


private:
	// Private functions

	// Private variables and classes
	unsigned int romSize;
	bool bootStrap;
	uint8_t extRAM[4][0xBFFF - 0xA000 + 1];	// Somewhat temporary
	ifstream romFileStream;
	// MBC1 registers
	bool ramEnable;
	int romBankNumber;
	int romRamBankNumber = 0;
	bool romRamMode;

	// Dynamically allocated data pointers
	uint8_t *romData;

};

