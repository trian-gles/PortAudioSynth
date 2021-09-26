#pragma once

#include <vector>
typedef std::vector<float> waveTable;

class BaseSound // class to be inherited of any sound that connects a voltage
{
public:
	virtual float GetSample();
};


class WavePlayer : public BaseSound
{
private:
	waveTable* sourceWave;
	int index = 0;
public:
	WavePlayer(waveTable* sourceWave);
	float GetSample() override;
};

waveTable* MakeHannTable(int samples);

waveTable* MakeLineTable(float start, float finish, int length);

class Grain : public BaseSound
{
private:
	waveTable* sourceWave;
	waveTable* window;
	int start;
	int finish;
	int index; // index for the soundfile
	int length;
	bool playing;

public:
	Grain(waveTable* sourceWave, int start, int finish, waveTable* window);
	float GetSample() override;
	bool IsPlaying();
	void UpdateParams(int newStart, int newFinish);
	void Play();

};


class GranularSynth : public BaseSound 
{
private:
	waveTable* sourceWave;
	waveTable* window;
	int start;
	int finish;
	int density;
	int index = 0;
	std::vector<Grain*>* grains = new std::vector<Grain*>();

public:
	GranularSynth(waveTable* sourceWave, int start, int finish, int density, waveTable* window);
	void UpdateParams(int start, int finish, int density);
	float GetSample() override;

};


class BaseSimpleFilter : public BaseSound
{
protected:
	float lastInput = 0;
	BaseSound* inputSig;
};


class SimpleLP : public BaseSimpleFilter
{
public:
	SimpleLP(BaseSound* input);

	float GetSample() override;
};


class SimpleHP : public BaseSimpleFilter
{
public:
	SimpleHP(BaseSound* input);

	float GetSample() override;
};


class SimpleFir : public BaseSimpleFilter
{
private:
	waveTable* pastInputs = new waveTable();
	waveTable* coefs;
	int order;

public:
	SimpleFir(BaseSound* input, int order, waveTable* coefs);

	float GetSample() override;
};


class Sig : BaseSound // constant float voltage
{
	float amp;
public:
	Sig(float initAmp);

	float GetSample() override;
};


class Noise : BaseSound // needs added parameter for mul and maybe table size? 
{
public:
	float GetSample() override;

};


class BaseSynth : public BaseSound // class to be inherited
{
protected:
	waveTable* table = new waveTable();
private:

	float phase = 0;
	float add = 0;
	BaseSound* mulInp;
	BaseSound* freqInp;

	void AdvancePhase();

	float GetIncrement();

public:
	void Init(BaseSound* freq, BaseSound* mul, float add);

	float GetSample() override;
};


class Saw : public BaseSynth
{
private:
	void BuildTable(int tabSize);

public:
	Saw(BaseSound* freq, BaseSound* mul, int tabSize, float add);

	Saw(float freq, BaseSound* mul, int tabSize, float add);

	Saw(BaseSound* freq, float mul, int tabSize, float add);

	Saw(float freq, float mul, int tabSize, float add);

};


class Sine : public BaseSynth
{
private:
	void BuildTable(int tabSize);

public:
	Sine(BaseSound* freq, BaseSound* mul, int tabSize, float add);

	Sine(float freq, BaseSound* mul, int tabSize, float add);

	Sine(BaseSound* freq, float mul, int tabSize, float add);

	Sine(float freq, float mul, int tabSize, float add);
};