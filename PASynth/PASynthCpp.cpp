#include "portaudio.h"
#include <iostream>
#include <math.h>
#include <cmath>
#include <vector>
#define NUM_SECONDS   (4)
#define SAMPLE_RATE   (48000)
#define PI 3.14159265f

class BaseSynth // class to be inherited
{
protected:
	std::vector<float>* table = new std::vector<float>();
private:
	
	float phase = 0;
	float increment;
	bool mulInp = false;
	bool freqInp = false;

	void AdvancePhase()
	{
		phase += increment;
		while (phase >= (float) table->size() - 1)
		{
			
			phase -= (float)table->size() - 1;
			
		}
	}

public:
	void FixedInit(float freq, float mul, int tabSize)
	{
		increment = tabSize * freq / SAMPLE_RATE;
	}

	float GetSample()
	{
		int intPhase = (int)round(phase);
		float samp = (*table)[intPhase];
		AdvancePhase();
		return samp;
	}
};

class Saw : BaseSynth
{
public:
	Saw(float freq, float mul, int tabSize)
	{
		FixedInit(freq, mul, tabSize);
		float inc = 2.0f / tabSize;
		for (int i = 0; i < tabSize; i++)
		{
			(*table).push_back(inc * i - 2.0f);
		}
		
	}
};

class PaWrapper 
{
private:
	BaseSynth *outputSynth;
	PaStream* stream;

	int paCallbackMethod(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags)
	{
		float* out = (float*)outputBuffer;
		unsigned long i;

		(void)timeInfo; /* Prevent unused variable warnings. */
		(void)statusFlags;
		(void)inputBuffer;

		for (i = 0; i < framesPerBuffer; i++)
		{
			float samp = outputSynth->GetSample();
			*out++ = samp;
			*out++ = samp;
		}

		return paContinue;
	}

	static int paCallback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData)
	{
		/* Here we cast userData to Sine* type so we can call the instance method paCallbackMethod, we can do that since
		   we called Pa_OpenStream with 'this' for userData */
		return ((PaWrapper*)userData)->paCallbackMethod(inputBuffer, outputBuffer,
			framesPerBuffer,
			timeInfo,
			statusFlags);
	}
public:
	PaWrapper(BaseSynth* out)
	{
		outputSynth = out;
	}

	PaError Init()
	{
		std::cout << "init stream\n";
		return Pa_Initialize();
	}

	PaError OpenStream()
	{
		std::cout << "opening stream\n";
		return Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, SAMPLE_RATE, 256, &PaWrapper::paCallback, this);
	}

	PaError RunStream()
	{
		PaError err;
		std::cout << "running stream\n";
		err = Pa_StartStream(stream);
		if (err != paNoError)
		{
			return err;
		}
		Pa_Sleep(NUM_SECONDS * 1000);
		return Pa_StopStream(stream);

	}

	PaError CloseStream()
	{
		std::cout << "closing stream\n";
		return Pa_CloseStream(stream);
	}

	void Terminate()
	{
		Pa_Terminate();
	}


};

int main(void)
{
	PaError err;
	Saw* synth = new Saw(440, 0.05f, 2000);
	PaWrapper *pa = new PaWrapper((BaseSynth*)synth);


	err = pa->Init();
	if (err != paNoError)
	{
		return err;
	}
	err = pa->OpenStream();
	if (err != paNoError)
	{
		return err;
	}
	err = pa->RunStream();
	if (err != paNoError)
	{
		return err;
	}
	err = pa->CloseStream();
	if (err != paNoError)
	{
		return err;
	}
	pa->Terminate();
	return 0;

}