#include "Cartridge.h"
#include "NOMBC.h"
#include "MBC1.h"
#include "MBC3.h"
#include "MBC5.h"


Cartridge::Cartridge()
{
}


Cartridge::~Cartridge()
{
}

Cartridge *Cartridge::getCartridge(string romPath)
{
	ifstream romFileStream(romPath, ios::in | ios::binary | ios::ate);
	int romSize;
	uint8_t *romData;

	if (romFileStream.is_open()) {
		// Create the data array
		romSize = (int)romFileStream.tellg();
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
		for (int i = 0; i < 0x8000; i++) {
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
	for (int i = 0; i < 16; i++) {
		title[i] = (char)romData[0x134 + i];
	}
	// Print
	cout << "Title: " << title << endl;
	// MBC Type
	int MBCType = (int)romData[0x147];

	// Determine RAM size early, incase the header defines it as being used
	int headerRAMSize = (int)romData[0x149];
	const unsigned int ramSizes[6] = { 0x0,0x800,0x2000,0x8000,0x20000,0x10000 };
	// Create a battery path string in case we end up using it.
	string batteryPath = romPath;
	batteryPath.erase(batteryPath.find_last_of("."), string::npos);
	batteryPath.append(".sav");

	// Create a returnCart object as well
	Cartridge* returnCart = nullptr;

	cout << "MBC Type: ";
	switch (MBCType) {
	case 0x00:
		cout << "ROM ONLY";
		returnCart = new NOMBC(romData, romSize);
		break;
	// MBC1s
	case 0x01:
		cout << "MBC1";
		returnCart = new MBC1(romData, romSize, 0);
		break;
	case 0x02:
		cout << "MBC1+RAM";
		returnCart = new MBC1(romData, romSize, ramSizes[headerRAMSize]);
		break;
	case 0x03:
		cout << "MBC1+RAM+BATTERY";
		returnCart = new MBC1(romData, romSize, ramSizes[headerRAMSize]);
		returnCart->setBatteryLocation(batteryPath);
		break;
	// MBC3s
	case 0x0F:
		cout << "MBC3+TIMER+BATTERY";
		returnCart = new MBC3(romData, romSize, 0);	// Does this one have no onboard RAM?
		break;
	case 0x10:
		cout << "MBC3+TIMER+RAM+BATTERY";
		returnCart = new MBC3(romData, romSize, ramSizes[headerRAMSize]);
		returnCart->setBatteryLocation(batteryPath);
		break;
	case 0x11:
		cout << "MBC3";
		returnCart = new MBC3(romData, romSize, 0);
		break;
	case 0x12:
		cout << "MBC3+RAM";
		returnCart = new MBC3(romData, romSize, ramSizes[headerRAMSize]);
		break;
	case 0x13:
		cout << "MBC3+RAM+BATTERY";
		returnCart = new MBC3(romData, romSize, ramSizes[headerRAMSize]);
		returnCart->setBatteryLocation(batteryPath);
		break;
	// MBC5s
	case 0x19:
		cout << "MBC5";
		returnCart = new MBC5(romData, romSize, 0);
		break;
	case 0x1A:
		cout << "MBC5+RAM";
		returnCart = new MBC5(romData, romSize, ramSizes[headerRAMSize]);
		break;
	case 0x1B:
		cout << "MBC5+RAM+BATTERY";
		returnCart = new MBC5(romData, romSize, ramSizes[headerRAMSize]);
		returnCart->setBatteryLocation(batteryPath);
		break;
	case 0x1C:
		cout << "MBC5+RUMBLE";
		returnCart = new MBC5(romData, romSize, 0);
		break;
	case 0x1D:
		cout << "MBC5+RUMBLE+RAM";
		returnCart = new MBC5(romData, romSize, ramSizes[headerRAMSize]);
		break;
	case 0x1E:
		cout << "MBC5+RUMBLE+RAM+BATTERY";
		returnCart = new MBC5(romData, romSize, ramSizes[headerRAMSize]);
		returnCart->setBatteryLocation(batteryPath);
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
	
	// Print RAM size
	cout << "RAM Size: ";
	const char* sizes[7] = { "None", "2 KB", "8 KB", "32 KB", "128 KB", "64 KB", "Other" };
	if (headerRAMSize <= 0x4) {
		cout << sizes[headerRAMSize] << endl;
	}
	else {
		cout << sizes[6] << endl;
	}
	// Leave another space
	cout << "\n";
	//

	// Default to MBC1 if error
	if (returnCart == nullptr) {
		cout << "Unimplemented MBC detected. Default to MBC5 and see if it works (it might not).\n";
		returnCart = new MBC5(romData, romSize, ramSizes[headerRAMSize]);
	}
	return returnCart;
}

bool Cartridge::loadBatteryFile(uint8_t * extRAM, unsigned int ramSize, string inBatteryPath)
{
	bool success = false;	// Bool to return if file loaded fine
	// Attempt to load in a battery file
	ifstream batteryFileStream(inBatteryPath, ios::in | ios::binary | ios::ate);
	if (batteryFileStream.is_open()) {
		if (ramSize == (int)batteryFileStream.tellg()) {
			batteryFileStream.seekg(0, ios::beg);
			for (unsigned int i = 0; i < ramSize; i++) {
				char oneByte;
				batteryFileStream.read((&oneByte), 1);
				extRAM[i] = (uint8_t)oneByte;
			}
			success = true;
		}
		else {
			cout << "Error: Opened save file doesn't match defined RAM size. Battery will not be saved\n";	// Probaly handle this better
			for (unsigned int i = 0; i < ramSize; i++) {
				extRAM[i] = 0;
			}
			success = false;
		}
		batteryFileStream.close();
		return success;
	}
	else {
		// Load in 0 data if save couldn't be open, basically making a new file.
		// Doesn't really account for cases where it couldn't be opened but still exists.
		for (unsigned int i = 0; i < ramSize; i++) {
			extRAM[i] = 0;
		}
	}
}

void Cartridge::saveBatteryFile(uint8_t * extRAM, unsigned int ramSize, string inBatteryPath)
{
	// Create a file stream
	ofstream batteryFileStream(inBatteryPath, ios::out | ios::binary);
	if (batteryFileStream.is_open()) {
		for (unsigned int i = 0; i < ramSize; i++) {
			char oneByte = (char)extRAM[i];
			batteryFileStream.write(&oneByte, 1);
		}
		batteryFileStream.close();
		cout << "Write (should be?) successful\n";
	}
	else {
		cout << "Error: Could not open save file stream. File will not be saved\n";
	}
}
