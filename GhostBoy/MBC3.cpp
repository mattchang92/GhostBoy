#include "MBC3.h"



MBC3::MBC3(uint8_t* romData, int romSize):romData(romData), romSize(romSize)
{
}


MBC3::~MBC3()
{
	free(romData);
}

void MBC3::sendData(uint16_t address, uint8_t data)
{
	if (address >= 0x0000 && address <= 0x1FFF) {
		ramEnable = (data & 0xF) == 0xA;
	}
	else if (address >= 0x2000 && address <= 0x3FFF) {
		romBankNumber = data & 0x7F;
		if (romBankNumber == 0) {
			romBankNumber = 1;
		}
	}
	else if (address >= 0x4000 && address <= 0x5FFF) {
		RAMRTCselect = data % 0xD;
	}
	else if (address >= 0x6000 && address <= 0x7FFF) {
		// TODO: RTC
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnable) {
			if (RAMRTCselect < 4) {
				extRAM[RAMRTCselect][address & 0x1FFF] = data;
			}
			else {
				// TODO: RTC
			}
		}
	}
}

uint8_t MBC3::recieveData(uint16_t address)
{
	// ROM Area
	if (address >= 0x0000 && address <= 0x7FFF) {
		uint32_t returnAddress = address & 0x3FFF;
		if (address & 0x4000) {
			returnAddress |= romBankNumber << 14;
		}
		return romData[returnAddress];
	}
	// RAM Area
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnable) {
			if (RAMRTCselect < 4) {
				return extRAM[RAMRTCselect][address & 0x1FFF];
			}
			else {
				// TODO: RTC
			}
		}
	}
	return 0xFF;
}
