#include "WTFactory.h"



int WTFactory::calccycles(int WTlen, int fulllen) {
	
	float numcyclestest = (float) fulllen / (float) WTlen ;
	float numcyclestemp = fulllen / WTlen ;
	
	if (WTlen == fulllen) {
		return 1;
	}
	else if (numcyclestest == numcyclestemp) { // is integer
		return (int) numcyclestemp;
	}
	else return 0;
	
}
	
    
int WTFactory::makeWaveTable(MorphOsc *osc, FloatArray sample, float baseFrequency, int WFid)     {
	
	
	fft.clear();
    dest.clear();
    zeros.clear();
    

	dest.copyFrom(sample);
	
	fourier->fft(dest, fft);
	int fftoffs = fft.getSize();
	
	int ret;
	

	float topFreq = baseFrequency * 16.0 /sampleRate;
	
	

	for (int i=0; i<(NOF_BandLimWT); i++)  {
		fftoffs /= 2;
		fft.setMagnitude(zeros, fftoffs, (fft.getSize())-fftoffs);
		tmp.copyFrom(fft);
		fourier->ifft(tmp, dest);
		ret = osc->addWaveTable(dest.getSize(), dest.getData(), topFreq, WFid, NOF_SAMPLES);
		topFreq *= 2.0;
		
	}
	

	return 0;
}



void WTFactory::makeMatrix(MorphOsc *osc, FloatArray fullsample, float baseFrequency)     {
	
	FloatArray tempsample;
	
	for (int WFid=0 ; WFid<NOF_SAMPLES ; WFid++) {
	tempsample = fullsample.subArray(WFid*SAMPLE_LEN, SAMPLE_LEN);
	makeWaveTable(osc, tempsample, baseFrequency, WFid);
	} 
    //return 0;
}
