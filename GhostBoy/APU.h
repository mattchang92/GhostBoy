#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "SDL.h"
#include "SquareChannel.h"
#include "WaveChannel.h"

#define samplesize 512

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
	// The 4 channels
	SquareChannel squareOne;
	SquareChannel squareTwo;
	WaveChannel waveChannel;
	//APUChannel squareOne;
	int frameSequenceCountDown = 8192;
	int downSampleCount = 95;
	int bufferFillAmount = 0;
	float squareBuffer0[samplesize] = { 0 };
	float squareBuffer1[samplesize] = { 0 };
	float waveBuffer[samplesize] = { 0 };
	bool buffer0Ready = false;
	bool buffer1Ready = false;
	unsigned int buffer0Amount = 0;
	unsigned int buffer1Amount = 0;
	unsigned int currentBuffer = 0;
	//uint8_t* currentBuffer = squareBuffer1;
	//uint8_t* currentBufferFinished = NULL;
	int stepCount = 0;
	bool bufferReady = false;
	uint8_t frameSequencer = 0;
	//ofstream foo;
};

