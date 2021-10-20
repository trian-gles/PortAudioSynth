#include "portaudio.h"
#include <iostream>
#include "AudioMath.h"
#include "WavFile.h"
#include "osc.h"
#include <string>
#include <thread>
#define SAMPLE_RATE   (48000)
#define NUM_SECONDS   (30000)
#define PORT 12000

class PaWrapper
{
private:
	BaseSound* outputSound;
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
			float samp = outputSound->GetSample();
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
	PaWrapper(BaseSound* out)
	{
		outputSound = out;
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

	void SetSound(BaseSound* sound)
	{
		outputSound = sound;
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

void ListenerThread(UdpListeningReceiveSocket *s)
{
	s->RunUntilSigInt();
}

int main(void)
{
	std::string filename = "C:/Users/bkier/source/repos/PASynth/demo.wav";

	std::vector<float>* waveform = GetWaveform(filename);
	waveTable* hann = MakeHannTable(5000);


	waveTable* startTab = MakeLineTable(850000, 1400000, 29100);
	waveTable* lengthTab = MakeNoiseTable(99000);
	MulTable(lengthTab, 2500);
	AddTable(lengthTab, 0);
	

	waveTable* densityTab = MakeLineTable(3000, 210, 70000);


	WavePlayer* startPl = new WavePlayer(startTab);
	WavePlayer* lengthPl = new WavePlayer(lengthTab);
	Sig* densSig = new Sig(3000);
	WavePlayer* densPl = new WavePlayer(densityTab);

	//MovingGranularSynth *granSynth = new MovingGranularSynth(waveform, (BaseSound*)startPl, (BaseSound*)lengthPl, (BaseSound*)densSig, hann);
	PaError err;
	/*std::vector<Sine*>* synths = new std::vector<Sine*>();
	Sine* freq = new Sine(70, 100, 3, 490.0f);
	PrintSamples(freq, 100);
	Sine* dua = new Sine(freq, 0.2f, 2000, 0);*/

	//Noise *whiteNoise = new Noise();
	//SimpleFir* fir = new SimpleFir((BaseSound*)whiteNoise, 4, new std::vector<float>{ 0.1, 0.3, 0.4, 0.1, 0.1 });
	ExamplePacketListener listener;
	UdpListeningReceiveSocket s(
		IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT),
		&listener);
	std::thread osc(ListenerThread, &s);

	GranularSynth* granSynth = new GranularSynth(waveform, 120000, 150500, 900, hann);
	granSynth->AddExtCtrl(listener.storedMessage);

	WavePlayer* wf = new WavePlayer(waveform);
	PaWrapper* pa = new PaWrapper((BaseSound*)granSynth);

	
	
	

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