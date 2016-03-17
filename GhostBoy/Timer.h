#include "stdint.h"
#include "Interrupts.h"

#pragma once
class Timer
{
public:
	Timer(Interrupts &interrupts);
	~Timer();
	// Register access
	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);
	// Timer update function
	void updateTimers(int lastCycleCount);
private: 
	// Interrupts
	Interrupts *interrupts;
	// Timer registers
	uint8_t DIV = 0x00;
	uint8_t TIMA = 0x00;
	uint8_t TMA = 0x00;
	uint8_t TAC = 0x00;
	uint8_t IF = 0x00;
	// Cycle counters for the two timers
	int DIVCycleCount = 0;
	int TIMACycleCount = 0;
	// Simple function for getting clock rate
	int clockRate(int code);
};

