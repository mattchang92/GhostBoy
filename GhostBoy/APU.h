#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "SDL.h"
#include "SquareChannel.h"
#include "WaveChannel.h"
#include "NoiseChannel.h"

#define samplesize 1024

using namespace std;

#pragma once
class APU
{
public:
	APU();
	~APU();

	void sendData(uint16_t address, uint8_t data);
	uint8_t recieveData(uint16_t address);
	void step(int cycles);
	static void audio_callback(void *userdata, Uint8 *stream, int len);
	void playSound();
	int getBufferFillAmount();
	uint8_t* getSquareBuffer();
	bool getBufferReady();

private:
	// Values OR'd into register reads.
	const uint8_t readOrValues[23] = {  0x80,0x3f,0x00,0xff,0xbf,
										0xff,0x3f,0x00,0xff,0xbf,
										0x7f,0xff,0x9f,0xff,0xbf,
										0xff,0xff,0x00,0x00,0xbf,
										0x00,0x00,0x70 };
	// APU Universal registers
	bool vinLeftEnable = false;
	uint8_t leftVol = 0;
	bool vinRightEnable = false;
	uint8_t rightVol = 0;
	bool leftEnables[4] = { false };
	bool rightEnables[4] = { false };
	bool powerControl = false;

	// The 4 channels
	SquareChannel squareOne;
	SquareChannel squareTwo;
	WaveChannel waveChannel;
	NoiseChannel noiseChannel;
	//APUChannel squareOne;
	int frameSequenceCountDown = 8192;
	int downSampleCount = 95;
	int bufferFillAmount = 0;
	float mainBuffer[samplesize] = { 0 };
	uint8_t frameSequencer = 0;
};

