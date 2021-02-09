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
    dest128.clear();
    dest64.clear();
    zeros.clear();
    

	dest.copyFrom(sample);
	
	fourier->fft(dest, fft);
	int fftOffs = fft.getSize();
	
	int ret;
	

	float topFrequency = baseFrequency * 11.0 /sampleRate;
	
	/* adding wave from the bank */
	osc->addWaveTable(sample.getSize(), sample.getData(), topFrequency, WFid, NOF_SAMPLES);	
		
	/* adding half fft wave */
	topFrequency *= 2.0;
	fftOffs /= 2;
	tmp.copyFrom(fft);
	fourier->ifft(tmp.subaray(0,fftOffs), dest128);
	osc->addWaveTable(dest128.getSize(), dest128.getData(), topFrequency, WFid, NOF_SAMPLES);	
		
	/* adding quarter fft wave */
	topFrequency *= 2.0;
	fftOffs /= 2;
	tmp.copyFrom(fft);
	fourier->ifft(tmp.subaray(0,fftOffs), dest64);
	osc->addWaveTable(dest64.getSize(), dest64.getData(), topFrequency, WFid, NOF_SAMPLES);
	
	/* adding quarter fft wave */	
	while (topFrequency < 14000.0/sampleRate)	{   // must be < 20Khz for 
		topFrequency *= 2.0;
		fftOffs /= 2;
		fft.setMagnitude(zeros, fftOffs, (fft.getSize())-fftOffs);
		tmp.copyFrom(fft);
		fourier->ifft(tmp.subaray(0,fftOffs), dest64);
		osc->addWaveTable(dest64.getSize(), dest64.getData(), topFrequency, WFid, NOF_SAMPLES);
	}
	

	return topFrequency*sampleRate;
}



void WTFactory::makeMatrix(MorphOsc *osc, FloatArray fullsample, float baseFrequency)     {
	
	FloatArray tempsample;
	
	for (int WFid=0 ; WFid<NOF_SAMPLES ; WFid++) {
	tempsample = fullsample.subArray(WFid*SAMPLE_LEN, SAMPLE_LEN);
	makeWaveTable(osc, tempsample, baseFrequency, WFid);
	} 
    //return 0;
}
