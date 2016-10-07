#include "MBC3.h"


MBC3::MBC3(uint8_t* romData, unsigned int romSize, unsigned int ramSize, bool timerPresent):romData(romData), romSize(romSize), ramSize(ramSize), RTC(timerPresent)
{
	if (!RTC) {
		extRAM = new uint8_t[ramSize];
	}
	else {
		extRAM = new uint8_t[ramSize+48];
	}
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
		if (!latch && (data & 0x1) == 0x1) {
			latchTimer();
		}
		latch = (data & 0x1) == 0x1;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnable && ramSize > 0) {
			if (RAMRTCselect < 4) {
				extRAM[((address & 0x1FFF) | (RAMRTCselect << 13)) & (ramSize -1)] = data;
				ramNewData = true;
			}
			else {
				if (RAMRTCselect >= 0x8 && RAMRTCselect <= 0xC) {
					// Keep timer up-to-date before a write
					updateTimer();
					// I'm assuming writes here only reflect on the real time registers?
					switch (RAMRTCselect) {
						// Seconds
					case 0x8:
						realSecs = data;
						break;
						// Minutes
					case 0x9:
						realMins = data;
						break;
						// Hours
					case 0xA:
						realHours = data;
						break;
						// Days (lower)
					case 0xB:
						realDays = data;
						break;
						// Days (upper), halt, Day carry
					case 0xC:
						realDaysHi = data;
						break;
					}
				}
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
				switch (RAMRTCselect) {
					// Seconds
					case 0x8:
						return latchSecs;
						break;
					// Minutes
					case 0x9:
						return latchMins;
						break;
					// Hours
					case 0xA:
						return latchHours;
						break;
					// Days (lower)
					case 0xB:
						return latchDays;
						break;
					// Days (upper), halt, Day carry
					case 0xC:
						return latchDaysHi;
						break;
				}
			}
		}
	}
	return 0xFF;
}

void MBC3::setBatteryLocation(string inBatteryPath)
{
	battery = true;
	batteryPath = inBatteryPath;
	if (!RTC && !Cartridge::loadBatteryFile(extRAM, ramSize, batteryPath)) {
		battery = false;	// Disable battery if load wasn't sucessful;
	}
	// Hackish way of reading in timer values from file (will result in 1 error though).
	else if (RTC && Cartridge::loadBatteryFile(extRAM, ramSize+48, batteryPath)){
		// We'll have some RTC values available at the end of extRAM array.
		unsigned int offset = ramSize;
		// "Real" time
		// Thanks to whoever decided these should be 4-byte little endians
		realSecs = extRAM[offset];
		realMins = extRAM[offset+4];
		realHours = extRAM[offset+8];
		realDays = extRAM[offset+12];
		realDaysHi = extRAM[offset+16];
		// Latched time
		latchSecs = extRAM[offset+20];
		latchMins = extRAM[offset + 24];
		latchHours = extRAM[offset + 28];
		latchDays = extRAM[offset + 32];
		latchDaysHi = extRAM[offset + 36];
		// Saved unix time
		for (int i = 0; i < 8; i++) {
			currentTime |= (extRAM[offset + 40 + i]) << (8 * i);
		}
		// If the time is 0, it's probably a new file. Or you're from the 70s.
		if (currentTime <= 0) {
			currentTime = time(nullptr);
		}
		// Update the timer now
		updateTimer();
	}
	else {
		battery = false;
		cout << "\nERROR: Save File does not contain timer values...\n";
	}
}

void MBC3::saveBatteryData()
{
	if (battery && !RTC) {
		Cartridge::saveBatteryFile(extRAM, ramSize, batteryPath);
	}
	else if (battery && RTC) {
		// Update timer
		updateTimer();
		// Put timer values on array before writing
		unsigned int offset = ramSize;

		// Real time
		extRAM[offset] = realSecs;
		extRAM[offset + 4] = realMins;
		extRAM[offset + 8] = realHours;
		extRAM[offset + 12] = realDays;
		extRAM[offset + 16] = realDaysHi;
		// Latched
		extRAM[offset + 20] = latchSecs;
		extRAM[offset + 24] = latchMins;
		extRAM[offset + 28] = latchHours;
		extRAM[offset + 32] = latchDays;
		extRAM[offset + 36] = latchDaysHi;
		// Unix time
		for (int i = 0; i < 8; i++) {
			extRAM[offset + 40 + i] = (uint8_t)(currentTime >> (8 * i));
		}
		Cartridge::saveBatteryFile(extRAM, ramSize+48, batteryPath);
	}
}

void MBC3::updateTimer()
{
	// Get the new unix time
	time_t newTime = time(nullptr);
	// Time difference in seconds
	unsigned int difference = 0;
	// Return if we either are trying to "travel back in time" or if timer halt bit is set
	// Update the stored timer value in either case
	if (newTime > currentTime && (realDaysHi & 0x40) != 0x40) {
		difference = (unsigned int)(newTime - currentTime);
		currentTime = newTime;
	}
	else {
		currentTime = newTime;
		return;
	}
	// Perform the incrementing calculations, on the "realtime" values.
	// Hope my idea is right here
	// First seconds
	unsigned int newSeconds = realSecs + difference;
	realSecs = newSeconds % 60;
	// Minutes
	unsigned int newMins = realMins + (newSeconds / 60);
	realMins = newMins % 60;
	// Hours
	unsigned int newHours = realHours + (newMins / 60);
	realHours = newHours % 24;
	// Days
	// Accounts for high bit.
	unsigned int newDays = (((realDaysHi & 0x1) << 8)|realDays) + (newHours / 24);
	realDays = newDays;	// Low 8-bits applies
	// High bit on days counter
	realDaysHi &= 0xFE;
	realDaysHi |= (newDays >> 8) & 0x1;	// Applies the 8th bit
	// Overflow bit
	if (newDays > 511) {
		realDaysHi |= 0x80;
	}

}

void MBC3::latchTimer()
{
	updateTimer();
	latchSecs = realSecs;
	latchMins = realMins;
	latchHours = realHours;
	latchDays = realDays;
	latchDaysHi = realDaysHi;
}
