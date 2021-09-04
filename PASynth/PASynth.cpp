#include "portaudio.h"
#include <iostream>
#define NUM_SECONDS   (4)
#define SAMPLE_RATE   (48000)

typedef struct
{
	float left_phase;
	float right_phase;
}
paTestData;

static int patestCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags, void *userData)
{
	// Cast data passed through stream to our structure
	paTestData* data = (paTestData*)userData;
	float *out = (float*)outputBuffer;
	unsigned int i;
	(void)inputBuffer; // not using this but don't want an error

	for (i = 0; i < framesPerBuffer; i++) {
		*out++ = data->left_phase;  /* left */
		*out++ = data->right_phase;  /* right */

		//simple sawtooth phaser
		data->left_phase += 0.01f;
		if (data->left_phase >= 1.0f) { data->left_phase -= 2.0f; }
		data->right_phase += 0.01f;
		if (data->right_phase >= 1.0f) { data->right_phase -= 2.0f; }
	}

	return 0;
}
//***************************************

int PrintError(PaError err)
{
	std::cout << "some error idk man";
	return err;
}

static paTestData data;
int main(void) {
	PaStream* stream;
	PaError err;

	printf("PortAudio Test: sawtooth \n");

	data.left_phase = data.right_phase = 0;
	err = Pa_Initialize();
	printf("Initialization successful \n");
	if (err != paNoError) {
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
	}



	int numDevices;
	numDevices = Pa_GetDeviceCount();
	if (numDevices < 0)
	{
		printf("ERROR: Pa_CountDevices returned 0x%x\n", numDevices);
		err = numDevices;
		if (err != paNoError) return PrintError(err);
	}
	std::cout << "Device count retrieved \n";
	const   PaDeviceInfo* deviceInfo;
	for (int i = 0; i < numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		std::cout << i << "\n";
		std::cout << deviceInfo->name << "\n";
	}

	std::cout << "All devices listed \n";
	
	err = Pa_OpenDefaultStream( &stream, 0, 2, paFloat32, SAMPLE_RATE, 256, patestCallback, &data);
	if (err != paNoError) return PrintError(err);

	err = Pa_StartStream(stream);
	if (err != paNoError) return PrintError(err);

	Pa_Sleep(NUM_SECONDS * 1000);
	err = Pa_StopStream(stream);
	if (err != paNoError) return PrintError(err);
	err = Pa_CloseStream(stream);
	if (err != paNoError) return PrintError(err);
	Pa_Terminate();
	printf("Test finished.\n");
	return err;
	

	
}