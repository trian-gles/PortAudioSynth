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
			if ((samp > 1) || (samp < -1))
			{
				printf("Clipping : %f\n", samp);
			}
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
		return Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, SAMPLE_RATE, 32, &PaWrapper::paCallback, this);
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


	PaError err;
	//ExamplePacketListener listener;
	//UdpListeningReceiveSocket s(
	//	IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT),
	//	&listener);
	//std::thread osc(ListenerThread, &s);

	SRand* startRand = new SRand(0, 1015, 1996, .2);
	SRand* delayRand = new SRand(0, 200, 500, 0.5);
	SRand* rateRand = new SRand(-0.01, 0, 0.1, 2);

	SGranSynth* granSynth = new SGranSynth(waveform, 120000, 127000, 1, 50, GranularSynth::windowType::hann, startRand, delayRand, rateRand);

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