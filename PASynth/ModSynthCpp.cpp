#include "portaudio.h"
#include <iostream>
#include <math.h>
#include <cmath>
#include <vector>
#define NUM_SECONDS   (4)
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

class Sig : BaseSound
{
	float amp;
public:
	Sig(float initAmp)
	{
		amp = initAmp;
	}

	float GetSample() override
	{
		return amp;
	}
};

class BaseSynth : public BaseSound // class to be inherited
{
protected:
	std::vector<float>* table = new std::vector<float>();
private:

	float phase = 0;
	float add = 0;
	BaseSound *mulInp;
	BaseSound *freqInp;

	void AdvancePhase()
	{
		phase += GetIncrement();
		while (phase >= (float)table->size() - 1)
		{

			phase -= (float)table->size() - 1;

		}
	}

	float GetIncrement()
	{
		return table->size() * freqInp->GetSample() / SAMPLE_RATE;
	}

public: 
	void Init(BaseSound *freq, BaseSound *mul, float add)
	{
		mulInp = mul;
		freqInp = freq;
		this->add = add;
	}

	float GetSample() override
	{
		int intPhase = (int)round(phase);
		float samp = (*table)[intPhase];
		AdvancePhase();
		return samp * mulInp->GetSample() + add;
	}
};

class Saw : public BaseSynth
{
private:
	void BuildTable(int tabSize)
	{
		float inc = 2.0f / tabSize;
		for (int i = 0; i < tabSize; i++)
		{
			(*table).push_back((inc * i - 1.0f));
		}
	}

public:
	Saw(BaseSound *freq, BaseSound* mul, int tabSize, float add)
	{
		Init(freq, mul, add);
		BuildTable(tabSize);
	}

	Saw(float freq, BaseSound* mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), mul, add);
		BuildTable(tabSize);
	}

	Saw(BaseSound* freq, float mul, int tabSize, float add)
	{
		Init(freq, (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}

	Saw(float freq, float mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}

};

class Sine : public BaseSynth
{
private:
	void BuildTable(int tabSize)
	{
		float inc = PI * 2 / tabSize;
		for (int i = 0; i < tabSize; i++)
		{
			(*table).push_back(sin(inc * i));
		}
	}

public:
	Sine(BaseSound* freq, BaseSound* mul, int tabSize, float add)
	{
		Init(freq, mul, add);
		BuildTable(tabSize);
	}

	Sine(float freq, BaseSound* mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), mul, add);
		BuildTable(tabSize);
	}

	Sine(BaseSound* freq, float mul, int tabSize, float add)
	{
		Init(freq, (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}

	Sine(float freq, float mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}
};

class PaWrapper
{
private:
	BaseSynth* outputSynth;
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

	void SetSynth(BaseSynth* synth)
	{
		outputSynth = synth;
	}



};

void PrintSamples(BaseSound* sound, int num)
{
	std::cout << "printing samples: \n";
	for (int i = 0; i < num; i++)
	{
		std::cout << sound->GetSample() << "\n";
	}
}

int main(void)
{
	PaError err;
	std::vector<Sine*>* synths = new std::vector<Sine*>();
	Sine* freq = new Sine(119, 200, 3, 420.0f);
	PrintSamples(freq, 100);
	Sine* dua = new Sine(freq, 0.2f, 2000, 0);
	PaWrapper* pa = new PaWrapper((BaseSynth*)dua);

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