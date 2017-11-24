#include "SerialDevice.h"
#include <iostream>

#pragma once
class SerialHardware : 
	public SerialDevice
{
public:
	SerialHardware(Interrupts &interrupts, bool CGBMode);
	~SerialHardware();
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);
	bool getTransferRequest();
	void transferComplete();
	uint8_t getSBout();
	void setSBin(uint8_t newSB);
	bool getMaster();
	// More serial methodology
	void connectDevice(SerialDevice* connectedDevice);
	uint8_t clockDevice(uint8_t bit);
	void clock(int cycles);


private:
	// SB Should technically interleave between gameboys
	SerialDevice* connectedDevice = NULL;
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

