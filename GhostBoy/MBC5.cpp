#include "MBC5.h"


MBC5::MBC5(uint8_t * romData, unsigned int romSize, unsigned int ramSize) :romData(romData), romSize(romSize), ramSize(ramSize)
{
	extRAM = new uint8_t[ramSize];
}

MBC5::~MBC5()
{
}

void MBC5::sendData(uint16_t address, uint8_t data)
{
	if (address >= 0x0000 && address <= 0x1FFF) {
		if (battery & ramEnable && (data & 0xF) != 0xA && ramNewData) {
			saveBatteryData();
			ramNewData = false;
		}
		ramEnable = (data & 0xF) == 0xA;
	}
	else if (address >= 0x2000 && address <= 0x2FFF) {
		romBankNumber = (romBankNumber & 0x100) | data;
	}
	else if (address >= 0x3000 && address <= 0x3FFF) {
		romBankNumber = (romBankNumber & 0xFF) | ((data & 0x1) << 9);
	}
	else if (address >= 0x3000 && address <= 0x5FFF) {
		ramBankNumber = data & 0xF;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnable && ramSize > 0) {
			extRAM[((address & 0x1FFF) | (ramBankNumber << 13)) & (ramSize - 1)] = data;
			ramNewData = true;
		}
	}
}

uint8_t MBC5::recieveData(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x7FFF) {
		uint32_t returnAddress = address & 0x3FFF;
		if (address & 0x4000) {
			returnAddress |= romBankNumber << 14;
		}
		return romData[returnAddress];
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnable && ramSize > 0) {
			return extRAM[((address & 0x1FFF) | (ramBankNumber << 13)) & (ramSize - 1)];
		}
	}
	return 0xFF;
}

void MBC5::setBatteryLocation(string inBatteryPath)
{
	battery = false;
	batteryPath = inBatteryPath;
	if (Cartridge::loadBatteryFile(extRAM, ramSize, batteryPath)) {
		battery = true;	// Disable battery if load wasn't sucessful;
	}
}

void MBC5::saveBatteryData()
{
	if (battery) {
		Cartridge::saveBatteryFile(extRAM, ramSize, batteryPath);
	}
}
