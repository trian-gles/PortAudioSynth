#include "portaudio.h"
#include <iostream>
#define NUM_SECONDS   (4)
#define SAMPLE_RATE   (48000)

typedef struct
{
	float table[1000];
	int left_phase;
	int right_phase;
}
paTestData;

float amp = 0.03f;

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
		*out++ = data->table[data->left_phase] * amp;  /* left */
		*out++ = data->table[data->right_phase] * amp;  /* right */

		//increment the phase
		size_t tabSize = sizeof(data->table) / sizeof(float);
		data->left_phase += 1;
		if (data->left_phase >= tabSize) { data->left_phase = 0; }
		data->right_phase += 1;
		if (data->right_phase >= tabSize) { data->right_phase = 0; }
	}

	return 0;
}

void FillSaw(paTestData* data)
{
	
	size_t tabSize = sizeof(data->table) / sizeof(float);
	float inc = 2.0f / (float)tabSize;
	for (size_t i = 0; i < tabSize; i++)
	{
		data->table[i] = -1.0f + (inc * i);
	}
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
	FillSaw(&data);

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