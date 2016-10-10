#include <stdint.h>

#pragma once
class WRAM
{
public:
	WRAM();
	~WRAM();
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);
private: 
	uint8_t WRAMBank = 1;
	uint8_t RAM[0x8000] = { 0 };
};

