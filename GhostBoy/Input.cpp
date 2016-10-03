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
	P1Data = (P1Data & 0xF)|(data & 0x30);
}

uint8_t Input::recieveData() {
	// TODO: Handle joypad interrupt (possibly) and STOP instruction
	uint8_t controlBits = 0;
	const uint8_t *keys = SDL_GetKeyboardState(NULL);

	// P14
	if ((P1Data & 0x30) == 0x20) {
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
	else if ((P1Data & 0x30) == 0x10){
		// A
		controlBits |= (keys[SDL_SCANCODE_S] ? 0 : 1) << 0;
		// B
		controlBits |= (keys[SDL_SCANCODE_A] ? 0 : 1) << 1;
		// Select
		controlBits |= (keys[SDL_SCANCODE_BACKSPACE] ? 0 : 1) << 2;
		// Start
		controlBits |= (keys[SDL_SCANCODE_RETURN] ? 0 : 1) << 3;
	}
	else {
		controlBits = 0xF;
	}
	P1Data &= 0xF0;
	P1Data |= controlBits;
	return P1Data|0xC0;
}