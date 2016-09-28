#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "SDL.h"
#include "SquareChannel.h"

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
	// The 4 channels
	SquareChannel squareOne;
	SquareChannel squareTwo;
	//APUChannel squareOne;
	int frameSequenceCountDown = 8192;
	int downSampleCount = 95;
	int bufferFillAmount = 0;
	float squareBuffer0[samplesize] = { 0 };
	float squareBuffer1[samplesize] = { 0 };
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

