#include<stdint.h>

#pragma once
class SquareChannel
{
public:
	SquareChannel();
	~SquareChannel();

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t data);
	void step();
	uint8_t getOutputVol();
	void lengthClck();
	void envClck();
	void sweepClck();
	bool getRunning();

private:
	// Trigger function
	void trigger();
	uint16_t sweepCalculation();
	// Duty Cycle table
	const bool dutyTable[4][8] = {
		{false, false, false, false, false, false, false, true},
		{true, false, false, false, false, false, false, true},
		{true, false, false, false, false, true, true, true},
		{false, true, true, true, true, true, true, false}
	};

	unsigned int outputVol = 0;
	unsigned int sequencePointer = 0;
	unsigned int duty = 0;
	int timer = 0;	// AKA frequency
	uint16_t timerLoad = 0;	// Reloads the timer
	bool lengthEnable = false;
	uint8_t volumeLoad = 0;
	uint8_t volume = 0;
	uint8_t lengthLoad = 0;
	uint8_t lengthCounter;
	bool envelopeAddMode = false;
	uint8_t envelopePeriodLoad = 0;
	int envelopePeriod = 0;
	// Sweep
	uint8_t sweepPeriodLoad = 0;
	int sweepPeriod = 0;
	bool sweepNegate = false;
	uint8_t sweepShift = 0;
	uint16_t sweepShadow = 0;
	bool sweepEnable = false;
	// Keeps track of last bit written to trigger
	bool triggerBit = false;

	bool enabled = false;
	bool dacEnabled = true;	// Seperate from the other enabled, controlled by high 5 bits of nr2
	bool envelopeRunning = true;	// 
};

