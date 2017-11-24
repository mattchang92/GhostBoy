#include "Printer.h"

Printer::Printer()
{
}


Printer::~Printer()
{
}

void Printer::connectDevice(SerialDevice* inConnectedDevice)
{
	connectedDevice = inConnectedDevice;
}

uint8_t Printer::clockDevice(uint8_t bit)
{
	uint8_t outGoingBit = 0;
	outGoingBit = (SB & 0x80) >> 7;
	SB <<= 1;
	SB |= bit;
	clockCount++;
	if (clockCount >= 8) {
		transferComplete();
		clockCount = 0;
	}

	// Return our SB bit
	return outGoingBit;
}

void Printer::clock(int cycles)
{
}

void Printer::transferComplete()
{
	// The GB Printer state machine is rather simple
	// After receiving two "magic bytes" ($88 and $33 in that order)
	// It will a particular amount of data to command the printer
	// Most of the time, 0 is returned, in some cases, something else is returned
	// After the magic bytes, every byte sent is added to a 16-bit checksum, and then checked
	switch (currentState) {
		// If the magic bytes are received, it'll move on
		case magicByte:
			if (SB == 0x88) {
				stateSteps = 1;
				SB = 0;
			}
			else if (SB == 0x33 && stateSteps == 1) {
				// Switch to the command step
				stateSteps = 0;
				currentState = command;
				SB = 0;
			}
			else {
				// Reset the state steps if invalid byte received
				stateSteps = 0;
				SB = 0;
			}
			break;
		case command:
			// Store the chosen command, move to compression Flag
			currentChecksum += SB;
			currentCommand = SB;
			currentState = compressionFlag;
			SB = 0;
			break;
		case compressionFlag:
			// Skip this state for now, I don't think anything really uses it
			currentChecksum += SB;
			currentState = dataLength;
			SB = 0;
			break;
		case dataLength:
			currentChecksum += SB;
			if (stateSteps == 0) {
				// Set the least significant byte
				commandDataLength = SB;
				stateSteps = 1;
				SB = 0;
			}
			else {
				// Set the most signficiant byte, switch state
				commandDataLength |= SB << 8;
				if (commandDataLength > 0) {
					currentState = commandData;
				}
				// If length is zero, skip to checksum state
				else {
					currentState = checksum;
				}
				stateSteps = 0;
				SB = 0;
				// Reset ram if requested
				if (currentCommand == 1) {
					memset(printerRam, 0, 8192 * sizeof(uint8_t));
					ramFillAmount = 0;
					printerRequest = false;
				}
			}
			break;
		case commandData:
			currentChecksum += SB;

			// Command 2 input, Palette is the most relevent, margins might also be implemented, another is related to exposure.
			if (currentCommand == 2) {
				if (stateSteps == 2) {
					printPalette = SB;
				}
			}
			// Command 4 input
			else if (currentCommand == 4) {
				printerRam[ramFillAmount] = SB;
				ramFillAmount = (ramFillAmount + 1) & 0x1FFF;
			}

			stateSteps++;
			// If we're done, move on
			if (stateSteps >= commandDataLength) {
				currentState = checksum;
				stateSteps = 0;
				// Request a print if a print was requested
				if (currentCommand == 2) {
					printerRequest = true;
					printRequested();
					// I think this is necessary for SMBDX? The relevant status bit wasn't documented well..
					ramFillAmount = 0;
				}
			}
			SB = 0;
			break;
		case checksum:
			if (stateSteps == 0) {
				compareChecksum = SB;
				stateSteps = 1;
				SB = 0;
			}
			else if (stateSteps == 1) {
				compareChecksum |= SB << 8;
				// Set the checksum pass boolean appropriately
				checksumPass = compareChecksum == currentChecksum;
				// Reset the stored checksum
				currentChecksum = 0;
				// Set the state
				// Change to alive indicator step, set SB for that step as well
				currentState = aliveIndicator;
				stateSteps = 0;
				SB = 0x81;
			}
			break;
		case aliveIndicator:
			// The "alive indicator" will have been sent, set the status into SB
			// Unless it was command 1, apparently?
			// Bit 7 - Too hot/cold (Emulator shouldn't have this issue!)
			// Bit 6 - Paper Jam (Be concerned if virtual paper somehow jams)
			// Bit 5 - Timeout occured (Documentation has no clue what this means? Could it be set if any timeout occurs?)
			// Bit 4 - Battery voltage too low (No battery!)
			// Bit 3 - Ready to print (Set if the buffer was filled with atleast $280 bytes)
			// Bit 2 - Printing request (Set when print starts, stays until command 1 is received)
			// Bit 1 - Currently Printing (Should be enough just to set to 0?)
			// Bit 0 - Checksum error (if the checksum pass is 0)
			// Possible corrections, 7 may be battery low, 6 may be hot/cold, 5 may be paper jam, 4 may be timeout
			if (currentCommand != 1) {
				SB = ((!checksumPass ? 1 : 0) << 0) | (0 << 1) | ((printerRequest ? 1 : 0) << 2) | ((ramFillAmount >= 0x280 ? 1 : 0) << 3) | (0 << 4) | (0 << 5) | (0 << 6) | (0 << 7);
				//SB = 0;
			}
			else {
				SB = 0;
			}
			currentState = status;
			break;
		case status:
			// Just reset back to the start
			currentState = magicByte;
			SB = 0;
			break;
	}
}

void Printer::printRequested()
{
	//unsigned int canvasHeight = (ramFillAmount * 4) / 160 + ((ramFillAmount * 4) % ramFillAmount != 0);
	// Some psuedo-code to describe the height of the canvas
	//numberOfTiles = ceil(ramFillAmount/16)
	//height ceil(numberOfTiles/20)*8
	// ceil might only work like this in C++11?
	unsigned int canvasHeight = (unsigned int)ceil(ceil(ramFillAmount/16.0)/20.0)*8;
	SDL_Surface* printout = SDL_CreateRGBSurface(0, 160, canvasHeight, 32, 0, 0, 0, 0);
	uint32_t* printoutPixels = (uint32_t*)printout->pixels;
	//i < (ramFillAmount * 8)/2
	for (int i = 0; i < (printout->w *printout->h); i++) {
		uint8_t tempByte = 0;
		unsigned int bufferX = i % 160;
		unsigned int bufferY = i / 160;
		unsigned int tileNum = (bufferX / 8)+((bufferY / 8)*20);
		unsigned int tileStartLocation = tileNum * 16;
		unsigned int tileX = bufferX % 8;
		unsigned int tileY = bufferY % 8;
		unsigned int byte0 = printerRam[(tileStartLocation + (tileY * 2)) & 0x1FFF];
		unsigned int byte1 = printerRam[(tileStartLocation + (tileY * 2) + 1) & 0x1FFF];
		tempByte = ((byte0 >> (7 - tileX)) & 0x1) | (((byte1 >> (7 - tileX)) & 0x1) << 1);
		// Chose the color based on the print palette (though it's rarely different than the standard layout)
		uint8_t color = (printPalette >> ((3 - tempByte))*2) & 0x3;
		printoutPixels[i] = colorTable[color];
	}

	// Save image with time-stamp
	time_t currentTime = time(nullptr);
	//tm* utcTime;
	//gmtime_s(&currentTime, utcTime);

	//char timeString[80]; 
	//strftime(timeString, 80, "GBPrinter_%G%m%d-%H%M%S.bmp", utcTime);
	std::string filePath = "GBPrinter_";
	filePath.append(std::to_string(currentTime));
	filePath.append(".bmp");
	SDL_SaveBMP(printout, filePath.c_str());
	SDL_FreeSurface(printout);
	std::cout << "Print saved as " << filePath << "\n";
}
