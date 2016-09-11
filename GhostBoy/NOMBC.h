#pragma once
#include "Cartridge.h"
class NOMBC :
	public Cartridge
{
public:
	NOMBC(uint8_t* romData, int romSize);
	~NOMBC();
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);

private:
	uint8_t* romData;
	unsigned int romSize;
};

