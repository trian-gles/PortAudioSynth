#include <math.h>
#include <cmath>
#include <vector>
#include "AudioMath.h"
#define SAMPLE_RATE   (48000)
#define PI 3.14159265f

float BaseSound::GetSample()
	{
		return 0.0f;
	}


SimpleLP::SimpleLP(BaseSound* input)
	{
		this->inputSig = input;
	}

float SimpleLP::GetSample()
	{
		float curInp = (inputSig->GetSample() * 0.5f) + lastInput * 0.5f;
		lastInput = curInp; // store the current input for the next sample frame
		return curInp;
	}


SimpleHP::SimpleHP(BaseSound* input)
	{
		this->inputSig = input;
	}

float SimpleHP::GetSample()
	{
		float curInp = (inputSig->GetSample() * 0.5f) - lastInput * 0.5f;
		lastInput = curInp; // store the current input for the next sample frame
		return curInp;
	}


SimpleFir::SimpleFir(BaseSound* input, int order, std::vector<float>* coefs)
	{
		this->coefs = coefs;
		inputSig = input;
		this->order = order;
		for (int i = 0; i < order; i++)
		{
			pastInputs->push_back(0);
		}
	}

float SimpleFir::GetSample()
	{
		float newSig = inputSig->GetSample() * (*coefs)[0];
		for (int i = 0; i < order; i++)
		{
			newSig += (*pastInputs)[i] * (*coefs)[i + 1]; // coefficients
		}
		pastInputs->insert(pastInputs->begin(), newSig);

		pastInputs->pop_back();
		return newSig;
	}


Sig::Sig(float initAmp)
	{
		amp = initAmp;
	}

float Sig::GetSample()
	{
		return amp;
	}



float Noise::GetSample()
	{
		return 1 - ((rand() % 200) / 100);
	}


void BaseSynth::AdvancePhase()
	{
		phase += GetIncrement();
		while (phase >= (float)table->size() - 1)
		{

			phase -= (float)table->size() - 1;

		}
	}

float BaseSynth::GetIncrement()
	{
		return table->size() * freqInp->GetSample() / SAMPLE_RATE;
	}


void BaseSynth::Init(BaseSound* freq, BaseSound* mul, float add)
	{
		mulInp = mul;
		freqInp = freq;
		this->add = add;
	}

float BaseSynth::GetSample()
	{
		int intPhase = (int)round(phase);
		float samp = (*table)[intPhase];
		AdvancePhase();
		return samp * mulInp->GetSample() + add;
	}


void Saw::BuildTable(int tabSize)
	{
		float inc = 2.0f / tabSize;
		for (int i = 0; i < tabSize; i++)
		{
			(*table).push_back((inc * i - 1.0f));
		}
	}

Saw::Saw(BaseSound* freq, BaseSound* mul, int tabSize, float add)
	{
		Init(freq, mul, add);
		BuildTable(tabSize);
	}

Saw::Saw(float freq, BaseSound* mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), mul, add);
		BuildTable(tabSize);
	}

Saw::Saw(BaseSound* freq, float mul, int tabSize, float add)
	{
		Init(freq, (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}

Saw::Saw(float freq, float mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}



void Sine::BuildTable(int tabSize)
	{
		float inc = PI * 2 / tabSize;
		for (int i = 0; i < tabSize; i++)
		{
			(*table).push_back(sin(inc * i));
		}
	}

Sine::Sine(BaseSound* freq, BaseSound* mul, int tabSize, float add)
	{
		Init(freq, mul, add);
		BuildTable(tabSize);
	}

Sine::Sine(float freq, BaseSound* mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), mul, add);
		BuildTable(tabSize);
	}

Sine::Sine(BaseSound* freq, float mul, int tabSize, float add)
	{
		Init(freq, (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}

Sine::Sine(float freq, float mul, int tabSize, float add)
	{
		Init((BaseSound*)new Sig(freq), (BaseSound*)new Sig(mul), add);
		BuildTable(tabSize);
	}