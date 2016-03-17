#include "Cartridge.h"



Cartridge::Cartridge(string romPath, bool bootStrap) : romFileStream(romPath, ios::in | ios::binary | ios::ate), bootStrap(bootStrap)
{
	// Make sure rom bank is 1
	romBankNumber = 1;
	if (romFileStream.is_open()) {
		// Create the data array
		int romSize = (int)romFileStream.tellg();
		romData = new uint8_t[romSize];
		romFileStream.seekg(0, ios::beg);	// Set file stream to beginning
		// Copy data to array
		for (int i = 0; i < romSize; i++) {
			char oneByte;
			romFileStream.read((&oneByte), 1);
			romData[i] = (uint8_t)oneByte;
		}
		romFileStream.close();
	}
	else {
		romData = new uint8_t[0x8000];
		for (int i = 0; i < 0x8000; i++){
			romData[i] = 0x00;
		}
		cout << "Rom could not be open. This probably won't work.";
	}
	// Print rom header info to console
	cout << "ROM Header Info:\n";
	// Gather info
	// Title
	char title[17];
	title[16] = 0x00;
	for (int i = 0; i < 16; i++){
		title[i] = (char)romData[0x134 + i];
	}
	// Print
	cout << "Title: " << title <<  endl;
	// MBC Type
	int MBCType = (int)romData[0x147];
	cout << "MBC Type: ";
	switch (MBCType) {
		case 0x00:
			cout << "ROM ONLY";
			break;
		case 0x01:
			cout << "MBC1";
			break;
		case 0x02:
			cout << "MBC1+RAM";
			break;
		case 0x03:
			cout << "MBC1+RAM+BATTERY";
			break;
		default:
			cout << "Other";
			break;
	}
	cout << endl;
	// Rom size
	int headerRomSize = (int)romData[0x148];
	if (headerRomSize <= 0x07) {
		headerRomSize = 32 << headerRomSize;
	}
	else {
		headerRomSize = 0xFF;
	}
	// Print
	cout << "ROM Size: ";
	if (headerRomSize != 0xFF) {
		cout << headerRomSize << " KB\n";
	}
	else {
		cout << "Other \n";
	}
	// RAM Size
	int headerRAMSize = (int)romData[0x149];
	// Print
	cout << "RAM Size: ";
	const char* sizes[7] = { "None", "2 KB", "8 KB", "32 KB", "128 KB", "64 KB", "Other" };
	if (headerRAMSize <= 0x4) {
		cout << sizes[headerRAMSize] << endl;
	}
	else {
		cout << sizes[6] << endl;
	}
	// Leave another space
	cout << endl;
	//
}


Cartridge::~Cartridge(){
	free(romData);
}

void Cartridge::sendData(uint16_t address, uint8_t input){
	if (address >= 0x0000 && address <= 0x1FFF) {
		if (input == 0x00) {
			ramEnable = false;
		}
		else if (input == 0x0A) {
			ramEnable = true;
		}
	}
	else if (address >= 0x2000 && address <= 0x3FFF) {
		if ((input & 0x1F) == 0) {
			input = 1;
		}
		romBankNumber &= 0xE0;	// Clear lower 5 bits
		romBankNumber |= input & 0x1F;
	}
	else if (address >= 0x4000 && address <= 0x5FFF) {
		// romRamMode: false = rom, true = ram
		if (romRamMode == false) {
			ramBankNumber = input & 0x3;
		}
		else if (romRamMode == true) {	// Little redundant
			romBankNumber &= 0x1F;	// Clear upper bits
			romBankNumber |= ((input & 0x3) << 6);
		}
	}
	else if (address >= 0x6000 && address <= 0x7FFF) {
		if (input == 0x00) {
			romRamMode = false;
		}
		else if (input == 0x01) {
			romRamMode = true;
		}
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnable) {
			extRam[ramBankNumber][address - 0xA000] = input;
		}
	}
}

uint8_t Cartridge::recieveData(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x7FFF) {
		if (address < 0x4000) {
			return romData[address];
		}
		else {
			return romData[(0x4000 * romBankNumber) + (address - 0x4000)];
		}
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnable) {
			return extRam[ramBankNumber][address - 0xA000];
		}
		else {
			return 0x00;
		}
	}
	else {
		// TODO: actual external ram
		return 0;
	}
}


