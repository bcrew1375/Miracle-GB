/*#include <sdl.h>
#include <stdio.h>
#include <memory.h>
#include <windows.h>
#include <math.h>
#include <fstream.h>

#include "define.h"

extern unsigned char *emu.memory.internRam];

#define SAMPLE_RATE 44100
#define CHANNELS    2

unsigned char Channel1Wave[4096];
unsigned char Channel2Wave[4096];
unsigned char Channel3Wave[4096];
unsigned char Channel4Wave[4096];

unsigned char PlayWave[4096];
unsigned char message[256];

unsigned char Channel1LengthCounter;
unsigned char Channel2LengthCounter;
unsigned char Channel3LengthCounter;
unsigned char Channel4LengthCounter;

unsigned short int Channel1GBFrequency;
unsigned short int Channel2GBFrequency;
unsigned short int Channel3GBFrequency;
unsigned short int Channel4GBFrequency;

unsigned short int Channel1Frequency;
unsigned short int Channel2Frequency;
unsigned short int Channel3Frequency;
unsigned short int Channel4Frequency;

unsigned short int Channel1BufferCounter;
unsigned short int Channel2BufferCounter;
unsigned short int Channel3BufferCounter;
unsigned short int Channel4BufferCounter;

unsigned short int Channel3BufferPos;

unsigned char Channel3VolumeLevel;

unsigned char samples[32];

unsigned char *streamPointer;

SDL_AudioSpec audio;

void emulateChannel1();
void emulateChannel2();
void emulateChannel3();
void emulateChannel4();
void mixChannels();
void soundCallback(void *userdata, unsigned char *stream, int len);

int InitializeSound();

FILE *romHandle;

long int bytesRead;
long int audioLen = 0;
unsigned long int audioPos = 0;

void soundCallback(void *userdata, unsigned char *stream, int len)
{
	//if (audioLen == 0)
	//	return;

	//len = (len > audioLen) ? audioLen : len;

	memcpy(stream, &PlayWave[0], len);

	//audioLen -= len;
	//audioPos += len;
}

int InitializeSound()
{
    audio.freq = SAMPLE_RATE;
	audio.format = AUDIO_U8;
	audio.channels = CHANNELS;
	audio.samples = 1024;
	audio.callback = soundCallback;
	audio.userdata = NULL;

	if(SDL_OpenAudio(&audio, NULL))
		return -1;

	Channel1BufferCounter = 0;
	Channel2BufferCounter = 0;
	Channel3BufferCounter = 0;
	Channel4BufferCounter = 0;
	
	Channel1GBFrequency = 0;
	Channel2GBFrequency = 0;
	Channel3GBFrequency = 0;
	Channel4GBFrequency = 0;
	
	Channel1Frequency = 0;
	Channel2Frequency = 0;
	Channel3Frequency = 0;
	Channel4Frequency = 0;

	Channel1LengthCounter = 0;
	Channel2LengthCounter = 0;
	Channel3LengthCounter = 0;
	Channel4LengthCounter = 0;
	
	Channel3BufferPos = 0;
	Channel3VolumeLevel = 0;
	
	memset(Channel1Wave, 0, 4096);
	memset(Channel2Wave, 0, 4096);
	memset(Channel3Wave, 0, 4096);
	memset(Channel4Wave, 0, 4096);
	memset(PlayWave, 0, 4096);

	romHandle = fopen("shutdown_sb.wav", "rb");
	bytesRead = fread(PlayWave, 1, 524288, romHandle);
	audioLen = bytesRead;
	fclose(romHandle);
	
	//SDL_PauseAudio(0);

	return 0;
}

void soundEmulation()
{
	/*memset(Channel1Wave, 0, 4096);
	memset(Channel2Wave, 0, 4096);
	memset(Channel3Wave, 0, 4096);
	memset(Channel4Wave, 0, 4096);
	memset(PlayWave, 0, 4096);
	memset(samples, 0, 32);
	//emulateChannel1();
	//emulateChannel2();
	emulateChannel3();
	//emulateChannel4();
}

void emulateChannel1()
{
}

void emulateChannel2()
{
}

void emulateChannel3()
{
	// See if the Length Counter should be decremented;
	if (IOregister_NR34 & BIT_6)
		IOregister_NR31--;

	if (IOregister_NR31 == 0)
	{
		// Turn off the Channel 3 status flag.
		IOregister_NR52 &= 255 - BIT_2;

		// Turn off the length counter enable flag.
		IOregister_NR34 &= 255 - BIT_6;
	}

	// See if the channel status flag is on.
	if (IOregister_NR52 & BIT_2)
	{
		// See if channel 3 is on.
		if (IOregister_NR30 & BIT_7)
		{
			// Get bits 5 and 6, these are the volume level.
			Channel3VolumeLevel = (IOregister_NR32 & 96) >> 5;

			// Get the channel's frequency.
			Channel3GBFrequency = ((IOregister_NR34 & 3) << 8) + IOregister_NR33;
		}
	}
}

void emulateChannel4()
{
}

void mixChannels()
{
}*/
