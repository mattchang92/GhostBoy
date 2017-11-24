#include <stdint.h>
#include <string>
#include <stdio.h>
#include <ctime>
#include <iostream>
#include "SerialDevice.h"
#include "SDL.h"

#pragma once
class Printer : 
	public SerialDevice
{
public:
	Printer();
	~Printer();
	// These are the important bits
	void connectDevice(SerialDevice* connectedDevice);	// Returns a pointer to itself
	uint8_t clockDevice(uint8_t bit);	// The printer never generates its own clock, it acts purely as a slave device
	// While the printer would never receive a clock signal beyond the Serial Clock, it may still be useful to track CPU cycles for relative timing (such as the printer's timeout)
	void clock(int cycles);
	
private:
	void transferComplete();
	void printRequested();
	// Printer ram
	uint8_t printerRam[8192] = { 0 };
	unsigned int ramFillAmount = 0;
	// Other
	int clockCount = 0;
	// Serial buffer thing
	uint8_t SB = 0;
	SerialDevice* connectedDevice = NULL;
	// Enum for current printer state
	enum State { magicByte, command, compressionFlag, dataLength, commandData, checksum, aliveIndicator, status};
	State currentState = magicByte;	// Initial state
	int stateSteps = 0;	 // Keeping track of progress into the current state
	uint8_t currentCommand = 0;
	uint16_t commandDataLength = 0;
	uint16_t currentChecksum = 0;
	uint16_t compareChecksum = 0;
	bool checksumPass = false;
	bool printerRequest = false;
	const uint32_t colorTable[4] = { 0x00000000, 0x404040, 0x808080, 0xFFFFFF};
	uint8_t printPalette = 0;
};

