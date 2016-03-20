#include "Timer.h"

// Timer addresses
#define DIVAddress 0xFF04
#define TIMAAddress 0xFF05
#define TMAAddress 0xFF06
#define TACAddress 0xFF07
#define IFAddress 0xFF0F
#define timerOverflowByte 0x04


Timer::Timer(Interrupts &interrupts) : interrupts(&interrupts)
{
}


Timer::~Timer()
{
}

void Timer::sendData(uint16_t address, uint8_t data) {
	switch (address) {
		case DIVAddress:
			DIV = 0;	// Any write to DIV sets it to 0
			break;
		case TIMAAddress:
			TIMA = data;
			break;
		case TMAAddress:
			TMA = data;
			break;
		case TACAddress:
			TAC = data;
			break;
		default:
			break;
	}
}

uint8_t Timer::recieveData(uint16_t address) {
	uint8_t returnVal = 0;
	switch (address) {
		case DIVAddress:
			returnVal = DIV;
			break;
		case TIMAAddress:
			returnVal = TIMA;
			break;
		case TMAAddress:
			returnVal = TMA;
			break;
		case TACAddress:
			returnVal = TAC;
			break;
		default:
			break;
	}
	return returnVal;
}

void Timer::updateTimers(int lastCycleCount) {
	// DIV is updated at a rate of 16384 times per second
	// No matter what
	// 4194304/16384 = 256 cycles for a DIV increment
	// Thanks realboy dev for that
	DIVCycleCount += lastCycleCount;
	if (DIVCycleCount >= 256) {
		DIVCycleCount -= 256;
		DIV++;
	}
	// TIMA is incremented by a set rate
	// TAC contains the rate it's set at,
	// And also whether or not it's allowed to update
	// When overflowed, an interrupt is called, and TIMA is replaced with TMA
	if ((TAC & 0x4) != 0) {
		TIMACycleCount += lastCycleCount;
		int clockRateNum = clockRate(TAC & 0x3);
		if (TIMACycleCount >= clockRateNum) {
			TIMACycleCount -= clockRateNum;
			if (TIMA == 0xff) {
				TIMA = TMA;
				interrupts->IF = interrupts->IF | (0xE0 | timerOverflowByte);
			}
			else {
				TIMA++;
			}
		}
	}
}

int Timer::clockRate(int code) {
	int returnVal = 0;
	switch (code) {
		case 0: 
			returnVal = 1024;
			break;
		case 1:
			returnVal = 16;
			break;
		case 2:
			returnVal = 64;
			break;
		case 3:
			returnVal = 256;
			break;
		default:
			break;
	}
	return returnVal;
}