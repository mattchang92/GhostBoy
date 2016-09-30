#include <stdint.h>

#pragma once
class NoiseChannel
{
public:
	NoiseChannel();
	~NoiseChannel();

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t data);
	void step();
	void envClck();
	void lengthClck();
	uint8_t getOutputVol();
	bool getRunning();

private:
	// Registers
	uint8_t lengthLoad = 0;
	uint8_t volumeLoad = 0;
	bool envelopeAddMode = false;
	uint8_t envelopePeriodLoad = 0;
	// Noise specific stuff
	uint8_t clockShift = 0;
	bool widthMode = false;
	uint8_t divisorCode = 0;
	bool triggerBit = false;
	bool lengthEnable = false;

	// Internal
	// Divisor table, for divisor codes
	const unsigned int divisors[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
	int timer = 0;
	bool enabled = false;
	bool dacEnabled = false;
	uint8_t lengthCounter = 0;
	uint8_t volume = 0;
	uint8_t envelopePeriod = 0;
	uint16_t lfsr = 0;	// Linear feedback shift register
	bool envelopeRunning = false;
	uint8_t outputVol = 0;
	void trigger();
};

