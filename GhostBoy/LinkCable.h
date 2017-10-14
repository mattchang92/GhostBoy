#include <stdint.h>
#include "Interrupts.h"

#pragma once
class SerialDevice
{
public:
	SerialDevice(Interrupts &interrupts, bool CGBMode);
	~SerialDevice();
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);
	bool getTransferRequest();
	void transferComplete();
	uint8_t getSBout();
	void setSBin(uint8_t newSB);
	bool getMaster();
	// More serial methodology
	SerialDevice connectDevice(SerialDevice &connectedDevice);	// Returns a pointer to itself
	uint8_t clockDevice(uint8_t bit);
	void clock(int cycles);


private:
	// SB Should technically interleave between gameboys
	SerialDevice *connectedDevice = NULL;
	Interrupts *interrupts;
	uint8_t SB = 0xFF;
	//uint8_t SC = 0xFF;
	// Use booleans for SC
	bool transferRequest = false;
	bool master = false;
	bool gbcDoubleSpeed = false;
	// Clock counter
	int clockCount = 0;
	int cpuCycles = 0;

	bool CGBMode = false;
};

