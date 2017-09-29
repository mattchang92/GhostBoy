#include <stdint.h>
#include "Interrupts.h"

#pragma once
class LinkCable
{
public:
	LinkCable(Interrupts &interrupts);
	~LinkCable();
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);
	bool getTransferRequest();
	void transferComplete();
	uint8_t getSBout();
	void setSBin(uint8_t newSB);
	bool getMaster();

private:
	// SB Should technically interleave between gameboys
	Interrupts *interrupts;
	uint8_t SB = 0xFF;
	uint8_t SC = 0xFF;
	bool transferStart = false;
	bool master = false;
};

