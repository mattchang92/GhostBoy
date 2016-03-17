#include "stdint.h"

#pragma once
class Interrupts
{
public:
	// Constructors
	Interrupts();
	~Interrupts();
	// Interrupt registers
	uint8_t IF = 0x00;
	uint8_t IE = 0x00;

};

