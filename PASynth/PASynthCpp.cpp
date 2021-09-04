#include "portaudio.h"
#include <iostream>
#include <math.h>
#include <cmath>
#include <vector>
#define NUM_SECONDS   (20)
#define SAMPLE_RATE   (48000)
#define PI 3.14159265f

class BaseSound // class to be inherited
{
public:
	virtual float GetSample()
	{
		return 0.0f;
	}
};

class BaseSynth : BaseSound // class to be inherited
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

	float GetSample() override
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

class Sine : BaseSynth
{
public:
	Sine(float freq, float mul, int tabSize)
	{
		FixedInit(freq, mul, tabSize);
		float inc = PI * 2 / tabSize;
		for (int i = 0; i < tabSize; i++)
		{
			(*table).push_back(sin(inc * i));
		}

	}
};

class Dua : BaseSound
{
private:

	std::vector<std::vector<float>*>* tables = new std::vector<std::vector<float>*>();
	std::vector<float> *increments = new std::vector<float>();
	std::vector<float>* curTab;
	std::vector<int>* maxCycles = new std::vector<int>();
	float curInc;
	float phase = 0;

	int cycles = 0;
	int curIndex = 0;
	int curMaxCycle;

	bool mulInp = false;
	bool freqInp = false;

	void CheckPhase()
	{
		if (cycles > curMaxCycle)
		{
			NextTable();
		}
		while (phase >= (float)curTab->size() - 1)
		{

			phase -= (float)curTab->size() - 1;
			cycles++;
		}
	}

	void AdvancePhase()
	{
		phase += curInc;
	}

	void NextTable()
	{
		if (curIndex == tables->size())
		{
			return;
		}
		curMaxCycle = (*maxCycles)[curIndex];
		curTab = (*tables)[curIndex];
		curInc = (*increments)[curIndex];
		curIndex += 1;
	}

public:
	Dua(float freq, float mul, int size)
	{
		for (int i = 0; i < size; i++)
		{
			int tabSize = 3 + size - i;
			float increment = tabSize * freq / SAMPLE_RATE;
			increments->push_back(increment);

			std::vector<float>* table = new std::vector<float>();
			float inc = PI * 2 / tabSize;
			for (int i = 0; i < tabSize; i++)
			{
				(*table).push_back(sin(inc * i));
			}
			tables->push_back(table);

			int maxCyc = (int) round(20000 / tabSize);
			maxCycles->push_back(maxCyc);
		}
		//build an empty table
		std::vector<float>* empty = new std::vector<float>();
		empty->push_back(0);
		empty->push_back(0);
		tables->push_back(empty);
		increments->push_back(0.5);
		maxCycles->push_back(1000);

		NextTable();
		std::cout << "Dua built \n";
		std::cout << "tables: \n";
		for (int i = 0; i < (int)tables->size(); i++)
		{
			std::cout << (*tables)[i]->size() << "\n";
		}
	}

	float GetSample() override 
	{
		CheckPhase();
		int intPhase = (int)round(phase);
		float samp = (*curTab)[intPhase];
		AdvancePhase();
		return samp;
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

	void SetSynth(BaseSynth *synth)
	{
		outputSynth = synth;
	}



};

int main(void)
{
	PaError err;
	std::vector<Sine*> *synths = new std::vector<Sine*>();
	Dua* dua = new Dua(140, 0.05f, 200);
	PaWrapper *pa = new PaWrapper((BaseSynth*) dua);


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