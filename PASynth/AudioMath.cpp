#include <math.h>
#include <cmath>
#include <vector>
#include "AudioMath.h"
#include <iostream>
#define SAMPLE_RATE   (48000)
#define PI 3.14159265f

float BaseSound::GetSample()
	{
		return 0.0f;
	}



/*
TABLES
*/

waveTable* MakeHannTable(int samples)
{
	waveTable* wf = new waveTable();
	for (int i = 0; i < samples; i++)
	{
		float samp = 0.5 - 0.5 * cos(2 * PI * i / samples);
		wf->push_back(samp);
	}
	return wf;
}

waveTable* MakeLineTable(float start, float finish, int length)
{
	waveTable* wf = new waveTable();
	float step = (finish - start) / length;
	for (int i = 0; i < length; i++)
	{
		wf->push_back(step* i + start);
	}

	return wf;
}

waveTable* MakeSineTable(int length)
{
	waveTable* wf = new waveTable();
	{
		float inc = PI * 2 / length;
		for (int i = 0; i < length; i++)
		{
			wf->push_back(sin(inc * i));
		}
	}
	return wf;
}

waveTable* MakeSawTable(int length)
{
	waveTable* wf = new waveTable();
	float inc = 2.0f / length;
	for (int i = 0; i < length; i++)
	{
		wf->push_back((inc * i - 1.0f));
	}
	return wf;
}

waveTable* MakeNoiseTable(int length)
{
	waveTable* wf = new waveTable();
	for (int i = 0; i < length; i++)
	{
		
		wf->push_back(1 - ((rand() % 200) / 100));
	}
	return wf;
}

void MulTable(waveTable* tab, float mul)
{
	for (int i = 0; i < tab->size(); i++)
	{

		(*tab)[i] *= mul;
	}
}

void AddTable(waveTable* tab, float add)
{
	for (int i = 0; i < tab->size(); i++)
	{

		(*tab)[i] += add;
	}
}

void ReverseTable(waveTable* tab)
{
	std::reverse(tab->begin(), tab->end());
}


WavePlayer::WavePlayer(waveTable* sourceWave)
{
	this->sourceWave = sourceWave;
}

float WavePlayer::GetSample()
{
	float newSamp = 0;
	if (index >= (int)sourceWave->size())
	{
		index = 0;
	}
	newSamp = (*sourceWave)[index];
	index++;
	return newSamp;
}





/*
GRANULAR SYNTHESIS
*/

Grain::Grain(std::vector<float>* sourceWave, int start, int finish, waveTable* window)
{
	this->start = start;
	this->finish = finish;
	this->length = finish - start;
	this->index = start;
	this->sourceWave = sourceWave;
	this->window = window;
	playing = true;
}

float Grain::GetSample() 
{
	float returnGrain = (*sourceWave)[index];
	this->finish;
	int absIndex = index - start;
 	int windowIndex = (int)round(absIndex * window->size() / length);
	float windowSamp = (*window)[windowIndex];
	returnGrain *= windowSamp;
	if (finish > start)
	{
		index++;
		if (index >= finish)
		{
			index = start;
			playing = false;
		}
	}
	else if (start > finish)
	{
		index--;
		if (index <= finish)
		{
			index = start;
			playing = false;
		}
	}
		
	return returnGrain;
}

void Grain::UpdateParams(int newStart, int newFinish)
{
	start = newStart;
	index = start;
	finish = newFinish;
	length = newFinish - newStart;
}

bool Grain::IsPlaying()
{
	return playing;
}

void Grain::Play()
{
	playing = true;
}

GranularSynth::GranularSynth(std::vector<float>* sourceWave, int start, int finish, int density, waveTable* window)
{
	this->sourceWave = sourceWave;
	this->window = window;
	this->start = start;
	this->finish = finish;
	this->density = density;
	grains->push_back(new Grain(sourceWave, start, finish, window));
}

float GranularSynth::GetSample() 
{
	if (index >= density)
	{
		if (oscCtrl)
		{
			if (storedParams[0] > 0)
			{
				//std::cout << "received '/wek/inputs' message in GranularSynth with arguments: " << storedParams[0] << " " << storedParams[1] << " " << storedParams[2] << "\n";
				int newStart = round(storedParams[0]);
				int newFinish = start + round(storedParams[1]);
				int newDens = round(storedParams[2]);

				UpdateParams(newStart, newFinish, newDens);
			}
			
		}

		//std::cout << "Need to start a grain" << std::endl;
		index = 0;
		bool foundNotPlayingGrain = false;
		for (size_t i = 0; i < grains->size(); i++)
		{
			if (!(*grains)[i]->IsPlaying())
			{
				(*grains)[i]->Play();
				(*grains)[i]->UpdateParams(start, finish); // will this unnecessary call slow me down?
				foundNotPlayingGrain = true;
				//std::cout << "Starting up stopped grain" << std::endl;
				break;
			}
		}
		if (!foundNotPlayingGrain)
		{
			//std::cout << "Making new grain" << std::endl;
			

			Grain* newGrain = new Grain(sourceWave, start, finish, this->window); // can I do this mid audio loop?
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

	index++;
	return fullOutput;
}

void GranularSynth::UpdateParams(int newStart, int newFinish, int density)
{
	start = newStart;
	finish = newFinish;
	this->density = density;
}

void GranularSynth::AddExtCtrl(float* storedParams)
{
	oscCtrl = true;
	this->storedParams = storedParams;
}

MovingGranularSynth::MovingGranularSynth(waveTable* sourceWave, BaseSound* startPlayer, BaseSound* lengthPlayer, BaseSound* densityPlayer, waveTable* window)
{
	this->sourceWave = sourceWave;
	this->window = window;
	this->startPlayer = startPlayer;
	this->lengthPlayer = lengthPlayer;
	this->densityPlayer = densityPlayer;


	this->start = (int)startPlayer->GetSample();
	this->finish = (int)lengthPlayer->GetSample() + this->start;
	if (this->finish == this->start)
	{
		this->finish++;
	}

	this->density = (int)densityPlayer->GetSample();
	grains->push_back(new Grain(sourceWave, start, finish, window));
}

float MovingGranularSynth::GetSample()
{
	this->start = (int)startPlayer->GetSample();
	this->finish = (int)lengthPlayer->GetSample() + this->start;
	if (this->finish == this->start)
	{
		this->finish++;
	}
	this->density = (int)densityPlayer->GetSample();
	return GranularSynth::GetSample();
}





/*
FILTERS
*/

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




/*
SIGS AND SYNTHS
*/

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