#include <stdint.h>

#pragma once
class WaveChannel
{
public:
	WaveChannel();
	~WaveChannel();

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t data);
	void step();
	void lengthClck();
	uint8_t getOutputVol();
	bool getRunning();

private:
	// Wave Table ram, 16 entries, 32 samples in total (4 bits per sample).
	uint8_t waveTable[16] = { 0 };
	// Registers
	uint8_t lengthLoad = 0;
	uint8_t volumeCode = 0;
	// Timer aka Frequency
	uint16_t timerLoad = 0;
	bool lengthEnable = false;
	bool triggerBit = false;
	// Internal
	uint8_t positionCounter = 0;
	uint16_t lengthCounter = 0;
	int timer = 0;
	uint8_t outputVol = 0;
	bool enabled = false;
	bool dacEnabled = false;
	void trigger();
};

