#include "MBC3.h"



MBC3::MBC3(uint8_t* romData, unsigned int romSize, unsigned int ramSize):romData(romData), romSize(romSize), ramSize(ramSize)
{
	extRAM = new uint8_t[ramSize];
}


MBC3::~MBC3()
{
	free(romData);
	free(extRAM);
}

void MBC3::sendData(uint16_t address, uint8_t data)
{
	if (address >= 0x0000 && address <= 0x1FFF) {
		// If it's being disabled and there's new data written to RAM, save the data and reset the newData flag
		if (battery & ramEnable && (data & 0xF) != 0xA && ramNewData) {
			saveBatteryData();
			ramNewData = false;
		}
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
		if (ramEnable && ramSize > 0) {
			if (RAMRTCselect < 4) {
				extRAM[((address & 0x1FFF) | (RAMRTCselect << 13)) & (ramSize -1)] = data;
				ramNewData = true;
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
			if (RAMRTCselect < 4 && ramSize > 0) {
				return extRAM[((address & 0x1FFF) | (RAMRTCselect << 13)) & (ramSize - 1)];
			}
			else {
				// TODO: RTC
			}
		}
	}
	return 0xFF;
}

void MBC3::setBatteryLocation(string inBatteryPath)
{
	battery = true;
	batteryPath = inBatteryPath;
	if (!Cartridge::loadBatteryFile(extRAM, ramSize, batteryPath)) {
		battery = false;	// Disable battery if load wasn't sucessful;
	}
}

void MBC3::saveBatteryData()
{
	Cartridge::saveBatteryFile(extRAM, ramSize, batteryPath);
}
