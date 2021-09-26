#include "sndfile.h"
#include "WavFile.h"

std::vector<float>* GetWaveform(std::string filename)
{
	std::vector<float>* waveForm = new std::vector<float>();
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));
	SNDFILE* f = sf_open(filename.c_str(), SFM_READ, &sfinfo);
	int readcount;
	float data;

	while ((readcount = (int)sf_read_float(f, &data, 1)))
	{
		waveForm->push_back(data);
	};

	return waveForm;
}