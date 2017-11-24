#include <stdint.h>
#include "Interrupts.h"

#pragma once
class SerialDevice
{
public:
	// Default constructor
	SerialDevice();
	~SerialDevice();
	// Main abstracts
	virtual void connectDevice(SerialDevice* connectedDevice) = 0;	// Returns a pointer to itself
	virtual uint8_t clockDevice(uint8_t bit) = 0;
	virtual void clock(int cycles) = 0;
};

