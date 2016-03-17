#include "Input.h"
#include "SDL.h"



Input::Input()
{
	P1Data = 0xFF;
	controlMode = false;
}


Input::~Input()
{
}

void Input::pollControl(uint8_t data) {
	P1Data = data;
	if ((P1Data & 0x10) == 0) {
		controlMode = false;
	}
	else if ((P1Data & 0x20) == 0) {
		controlMode = true;
	}
}

uint8_t Input::recieveData() {
	// TODO: Handle joypad interrupt (possibly) and STOP instruction
	uint8_t controlBits = 0;
	const uint8_t *keys = SDL_GetKeyboardState(NULL);

	// P14
	if (!controlMode) {
		// Right
		controlBits |= (keys[SDL_SCANCODE_RIGHT] ? 0 : 1) << 0;
		// Left
		controlBits |= (keys[SDL_SCANCODE_LEFT] ? 0 : 1) << 1;
		// Up
		controlBits |= (keys[SDL_SCANCODE_UP] ? 0 : 1) << 2;
		// Down
		controlBits |= (keys[SDL_SCANCODE_DOWN] ? 0 : 1) << 3;
	}
	// P15
	else {
		// A
		controlBits |= (keys[SDL_SCANCODE_X] ? 0 : 1) << 0;
		// B
		controlBits |= (keys[SDL_SCANCODE_Z] ? 0 : 1) << 1;
		// Select
		controlBits |= (keys[SDL_SCANCODE_BACKSPACE] ? 0 : 1) << 2;
		// Start
		controlBits |= (keys[SDL_SCANCODE_RETURN] ? 0 : 1) << 3;
	}
	P1Data &= 0xF0;
	P1Data |= controlBits;
	return P1Data;
}