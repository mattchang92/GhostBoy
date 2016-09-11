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
		returnCart = new MBC1(romData, romSize);
		break;
	case 0x02:
		cout << "MBC1+RAM";
		returnCart = new MBC1(romData, romSize);
		break;
	case 0x03:
		cout << "MBC1+RAM+BATTERY";
		returnCart = new MBC1(romData, romSize);
		break;
	// MBC3s
	case 0x0F:
		cout << "MBC3+TIMER+BATTERY";
		returnCart = new MBC3(romData, romSize);
		break;
	case 0x10:
		cout << "MBC3+TIMER+RAM+BATTERY";
		returnCart = new MBC3(romData, romSize);
		break;
	case 0x11:
		cout << "MBC3";
		returnCart = new MBC3(romData, romSize);
		break;
	case 0x12:
		cout << "MBC3+RAM";
		returnCart = new MBC3(romData, romSize);
		break;
	case 0x13:
		cout << "MBC3+RAM+BATTERY";
		returnCart = new MBC3(romData, romSize);
		break;
	// MBC5s
	case 0x19:
		cout << "MBC5";
		returnCart = new MBC5(romData, romSize);
		break;
	case 0x1A:
		cout << "MBC5+RAM";
		returnCart = new MBC5(romData, romSize);
		break;
	case 0x1B:
		cout << "MBC5+RAM+BATTERY";
		returnCart = new MBC5(romData, romSize);
		break;
	case 0x1C:
		cout << "MBC5+RUMBLE";
		returnCart = new MBC5(romData, romSize);
		break;
	case 0x1D:
		cout << "MBC5+RUMBLE+RAM";
		returnCart = new MBC5(romData, romSize);
		break;
	case 0x1E:
		cout << "MBC5+RUMBLE+RAM+BATTERY";
		returnCart = new MBC5(romData, romSize);
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

	// Default to MBC1 if error
	if (returnCart == nullptr) {
		cout << "Unimplemented MBC detected. Default to MBC5 and see if it works (it might not).\n";
		returnCart = new MBC5(romData, romSize);
	}
	return returnCart;
}
