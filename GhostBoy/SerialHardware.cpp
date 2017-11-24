#include "SerialHardware.h"



SerialHardware::SerialHardware(Interrupts &interrupts, bool CGBMode) : interrupts(&interrupts), CGBMode(CGBMode)
{
}


SerialHardware::~SerialHardware()
{
}

void SerialHardware::sendData(uint16_t address, uint8_t data)
{
	switch (address) {
		// SB
	case 0xFF01:
		SB = data;
		if (clockCount != 0 && master) {
			std::cout << "SB Set before transfer finished on master?\n";
		}
		if (clockCount != 0 && !master) {
			std::cout << "SB Set before transfer finished on slave?\n";
		}
		break;
		// SC
	case 0xFF02:
		// Check if Serial transfer was started
		transferRequest = (data & 0x80) == 0x80;
		gbcDoubleSpeed = (CGBMode && (data & 0x2) == 0x02);
		master = (data & 0x1) == 0x1;
		break;
	}
}

uint8_t SerialHardware::recieveData(uint16_t address)
{
	uint8_t returnData = 0xFF;
	switch (address) {
		// SB
	case 0xFF01:
		returnData = SB;
		break;
		// SC
	case 0xFF02:
		returnData = ((transferRequest ? 1 : 0) << 7) | ((gbcDoubleSpeed ? 1 : 0) << 1) | ((master ? 1 : 0));
		break;
	}
	return returnData;
}

// Bad Stuff
bool SerialHardware::getTransferRequest()
{
	bool returnVal = transferRequest;
	//transferRequest = false;
	return returnVal;
}

void SerialHardware::transferComplete()
{
	// Interrupt the system
	interrupts->IF = interrupts->IF | (0xE0 | 0x8);
	transferRequest = false;
}

uint8_t SerialHardware::getSBout()
{
	return SB;
}

void SerialHardware::setSBin(uint8_t newSB)
{
	SB = newSB;
}

bool SerialHardware::getMaster()
{
	return master;
}

// Good stuff
void SerialHardware::connectDevice(SerialDevice* connectedDeviceIn)
{
	connectedDevice = connectedDeviceIn;
}

uint8_t SerialHardware::clockDevice(uint8_t bit)
{
	uint8_t outGoingBit = 0;
	if (transferRequest) {
		outGoingBit = (SB & 0x80) >> 7;
		SB <<= 1;
		SB |= bit;
		clockCount++;
		if (clockCount >= 8) {
			transferComplete();
			clockCount = 0;
		}
	}

	// Return our SB bit
	return outGoingBit;
}

void SerialHardware::clock(int cycles)
{
	cpuCycles += cycles;
	// Amount of CPU cycles depends on bit 1 if this is a Color
	int cycleLimit = gbcDoubleSpeed ? 256 : 512;
	// Only clocks if set to master mode and requested a transfer
	while (cpuCycles >= cycleLimit) {
		cpuCycles -= cycleLimit;
		if (master && transferRequest) {
			uint8_t outGoingBit = (SB & 0x80) >> 7;
			SB <<= 1;
			if (connectedDevice != NULL) {
				SB |= connectedDevice->clockDevice(outGoingBit);
			}
			else {
				SB |= 1;
			}
			clockCount++;
			if (clockCount >= 8) {
				transferComplete();
				clockCount = 0;
			}
		}
	}

	/*if (master && transferRequest) {
	for (int i = 0; i < 8; i++) {
	SB >>= 1;
	if (connectedDevice != NULL) {
	uint8_t outGoingBit = SB & 0x1;
	SB |= (connectedDevice->clockDevice(outGoingBit)) << 7;
	}
	}
	transferComplete();
	}*/

	/*if (master && transferRequest) {
	cpuCycles += cycles;
	int cycleLimit = 512*8;
	if (cpuCycles >= cycleLimit) {
	cpuCycles -= cycleLimit;
	for (int i = 0; i < 8; i++) {
	uint8_t outGoingBit = SB & 0x1;
	SB >>= 1;
	if (connectedDevice != NULL) {
	SB |= (connectedDevice->clockDevice(outGoingBit)) << 7;
	}
	}
	transferComplete();
	// Direct swap
	uint8_t swap = SB;
	SB = connectedDevice->getSBout();
	connectedDevice->setSBin(swap);
	connectedDevice->transferComplete();
	transferComplete();
	}
	}*/
}
