#include <stdint.h>

#pragma once
class Input
{
public:
	Input(bool player2);
	~Input();
	uint8_t recieveData();
	void pollControl(uint8_t data);

private:
	uint8_t P1Data;
	bool controlMode;
	bool player2;
};

