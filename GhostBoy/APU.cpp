#include "APU.h"
#include <stdio.h>


APU::APU()// : foo("./output.bin", ios::out | ios::binary)
{
	// Set up SDL audio spec
	SDL_AudioSpec audioSpec;
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_F32SYS;
	audioSpec.channels = 1;
	audioSpec.samples = samplesize;	// Adjust as needed
	audioSpec.callback = NULL;
	audioSpec.userdata = this;

	SDL_AudioSpec obtainedSpec;
	SDL_OpenAudio(&audioSpec, &obtainedSpec);
	SDL_PauseAudio(0);
}


APU::~APU()
{
}

void APU::sendData(uint16_t address, uint8_t data)
{
	uint8_t apuRegister = address & 0xFF;
	// Pulse 1 redirect
	if (apuRegister >= 0x10 && apuRegister <= 0x14) {
		squareOne.writeRegister(apuRegister, data);
	}
	// Pulse 2 redirect
	// ignore 0x15 since sweep doesn't exist
	else if (apuRegister >= 0x16 && apuRegister <= 0x19) {
		squareTwo.writeRegister(apuRegister, data);
	}
}

uint8_t APU::recieveData(uint16_t address)
{
	uint8_t returnData = 0;
	return returnData;
}

void APU::step(int cycles)
{
	// 1 CPU Cycle = 1 APU cycle
	while (cycles-- != 0) {
		// Frame Sequencer
		// Ticks every 8192 CPU cycles (on a 512 scale, 4194304/512 = 8192).
		if (--frameSequenceCountDown <= 0) {
			frameSequenceCountDown = 8192;
			switch (frameSequencer) {
				case 0:
					squareOne.lengthClck();
					squareTwo.lengthClck();
					break;
				case 1:
					break;
				case 2:
					squareOne.sweepClck();
					squareOne.lengthClck();
					squareTwo.lengthClck();
					break;
				case 3:
					break;
				case 4:
					squareOne.lengthClck();
					squareTwo.lengthClck();
					break;
				case 5:
					break;
				case 6:
					squareOne.sweepClck();
					squareOne.lengthClck();
					squareTwo.lengthClck();
					break;
				case 7:
					squareOne.envClck();
					squareTwo.envClck();
					break;
			}
			frameSequencer++;
			if (frameSequencer >= 8) {
				frameSequencer = 0;
			}
		}

		// Step all the channels
		squareOne.step();
		squareTwo.step();

		if (--downSampleCount <= 0) {
			downSampleCount = 95;
			float bufferin1 = ((float)squareOne.getOutputVol()) / 100;
			float bufferin2 = ((float)squareTwo.getOutputVol()) / 100;
			SDL_MixAudioFormat((Uint8*)&bufferin1, (Uint8*)&bufferin2, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
			squareBuffer0[bufferFillAmount] = bufferin1;
			bufferFillAmount++;
		}
		if (bufferFillAmount >= samplesize) {
			bufferFillAmount = 0;
			// Delay execution until the SDL audio queue is mostly empty.
			while (SDL_GetQueuedAudioSize(1) > 15) {
				SDL_Delay(1);
			}
			SDL_QueueAudio(1, squareBuffer0, samplesize*sizeof(float));
		}
		stepCount++;	// I'd be worried about this overflowing
	}
}

// SDL audio callback
void APU::audio_callback(void* userdata, Uint8 * stream, int len)
{
	APU* apuData = (APU*)userdata;	// Um
	float* floatStream = (float*)stream;
	int floatLen = len / sizeof(float);
	// uuuuuuuuuuuuuuuuuuuuuu how do I implement this in a sane way?
	/*if (apuData->getBufferReady()) {
		//SDL_MixAudio(stream, NULL, 2048, SDL_MIX_MAXVOLUME);	// LEGACY METHOD

		uint8_t* squareBuffer = apuData->getSquareBuffer();
		for (int i = 0; i < floatLen; i++) {
			float output = squareBuffer[i];
			floatStream[i] = output/100;
		}
	}
	else {
		for (int i = 0; i < floatLen; i++) {
			floatStream[i] = 0;
		}
	}*/
	//SDL_PauseAudio(1);
}

void APU::playSound()
{
	/*if (currentBuffer == 1) {
		SDL_QueueAudio(1, squareBuffer0, samplesize);
		buffer0Ready = false;
	}
	else if (currentBuffer == 0){
		SDL_QueueAudio(1, squareBuffer1, samplesize);
		buffer1Ready = false;
	}*/
	/*if (buffer0Ready) {
		SDL_QueueAudio(1, squareBuffer0, samplesize);
		buffer0Ready = false;
	}
	if (buffer1Ready) {
		SDL_QueueAudio(1, squareBuffer1, samplesize);
		buffer1Ready = false;
	}
	if (!buffer0Ready && !buffer1Ready) {
		printf("Audio Skip ");
	}*/
	if (currentBuffer == 0) {
		SDL_QueueAudio(1, squareBuffer0, buffer0Amount);
		currentBuffer = 1;
		buffer1Amount = 0;
		bufferFillAmount = 0;
	}
	if (currentBuffer == 1) {
		SDL_QueueAudio(1, squareBuffer1, buffer1Amount);
		currentBuffer = 0;
		buffer0Amount = 0;
		bufferFillAmount = 0;
	}
}

int APU::getBufferFillAmount()
{
	return bufferFillAmount;
}

uint8_t * APU::getSquareBuffer()
{
	return NULL;
}

bool APU::getBufferReady()
{
	bool returnBool = bufferReady;
	bufferReady = false;
	return returnBool;
}
