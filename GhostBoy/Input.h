#include <stdint.h>

#pragma once
class Input
{
public:
	Input();
	~Input();
	uint8_t recieveData();
	void pollControl(uint8_t data);

private:
	uint8_t P1Data;
	bool controlMode;
};

