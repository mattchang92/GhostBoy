#include "LinkCable.h"

LinkCable::LinkCable(Interrupts & interrupts) : interrupts(&interrupts)
{
}

LinkCable::~LinkCable()
{
}

void LinkCable::sendData(uint16_t address, uint8_t data)
{
	switch (address) {
		// SB
		case 0xFF01:
			SB = data;
			break;
		// SC
		case 0xFF02:
			SC = data;
			// Check if Serial transfer was started
			if ((SC & 0x80) == 0x80) {
				transferStart = true;
			}
			if ((SC & 0x1) == 0x1) {
				master = true;
			}
			else {
				master = false;
			}
			break;
	}
}

uint8_t LinkCable::recieveData(uint16_t address)
{
	uint8_t returnData = 0xFF;
	switch (address) {
		// SB
	case 0xFF01:
		returnData = SB;
		break;
		// SC
	case 0xFF02:
		returnData = SC;
		break;
	}
	return returnData;
}

bool LinkCable::getTransferRequest()
{
	bool returnVal = transferStart;
	transferStart = false;
	return returnVal;
}

void LinkCable::transferComplete()
{
	// Interrupt the system
	interrupts->IF = interrupts->IF | (0xE0 | 0x8);
	SC &= 0x7F;
}

uint8_t LinkCable::getSBout()
{
	return SB;
}

void LinkCable::setSBin(uint8_t newSB)
{
	SB = newSB;
}

bool LinkCable::getMaster()
{
	return master;
}
