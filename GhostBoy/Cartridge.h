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

	static Cartridge* getCartridge(string romPath);
};

