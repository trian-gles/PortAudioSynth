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


WavePlayer::WavePlayer(std::vector<float>* sourceWave)
{
	this->sourceWave = sourceWave;
}

float WavePlayer::GetSample()
{
	float newSamp = 0;
	if ((size_t)index < sourceWave->size())
	{
		newSamp = (*sourceWave)[index];
	}
	index++;
	return newSamp;
}


Grain::Grain(std::vector<float>* sourceWave, int start, int finish)
{
	this->start = start;
	this->finish = finish;
	this->sourceWave = sourceWave;
	playing = true;
}

float Grain::GetSample() 
{
	float returnGrain = (*sourceWave)[index];
	index++;
	if (index == finish) 
	{
		index = start;
		playing = false;
	}
	return returnGrain;
}

void Grain::UpdateParams(int newStart, int newFinish)
{
	start = newStart;
	finish = newFinish;
}

bool Grain::IsPlaying()
{
	return playing;
}

void Grain::Play()
{
	playing = true;
}

GranularSynth::GranularSynth(std::vector<float>* sourceWave, int start, int finish, int density)
{
	this->sourceWave = sourceWave;
	this->start = start;
	this->finish = finish;
	this->density = density;
	grains->push_back(new Grain(sourceWave, start, finish));
}

float GranularSynth::GetSample() 
{
	if (index == density)
	{
		index = 0;
		bool foundNotPlayingGrain = false;
		for (size_t i = 0; i < grains->size(); i++)
		{
			if (!(*grains)[i]->IsPlaying())
			{
				(*grains)[i]->Play();
				(*grains)[i]->UpdateParams(start, finish); // will this unnecessary call slow me down?
				foundNotPlayingGrain = true;
				break;
			}
		}
		if (!foundNotPlayingGrain)
		{
			Grain* newGrain = new Grain(sourceWave, start, finish); // can I do this mid audio loop?
			grains->push_back(newGrain);
			newGrain->Play();
		}
	}

	float fullOutput = 0;

	for (size_t i = 0; i < grains->size(); i++) 
	{
		if ((*grains)[i]->IsPlaying())
		{
			fullOutput += (*grains)[i]->GetSample();
		}
		// should I clean up unneeded grains here?
	}

	return fullOutput;
}

void GranularSynth::UpdateParams(int newStart, int newFinish, int density)
{
	start = newStart;
	finish = newFinish;
	this->density = density;
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