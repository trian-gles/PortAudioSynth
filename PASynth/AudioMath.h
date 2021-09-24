#pragma once

#include <vector>


class BaseSound // class to be inherited of any sound that connects a voltage
{
public:
	virtual float GetSample();
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
	std::vector<float>* pastInputs = new std::vector<float>();
	std::vector<float>* coefs;
	int order;

public:
	SimpleFir(BaseSound* input, int order, std::vector<float>* coefs);

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
	std::vector<float>* table = new std::vector<float>();
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