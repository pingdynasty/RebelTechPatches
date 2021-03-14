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
	
    
int WTFactory::makeWaveTable(MorphOsc *osc, FloatArray sample, float baseFrequency, int WFidX, int WFidY)     {
	
	
	fft.clear();
    dest.clear();
    dest128.clear();
    dest64.clear();
    zeros.clear();
    

	dest.copyFrom(sample);
	
	fourier->fft(dest, fft);
	int fftOffs = fft.getSize();
	
	int ret;
	

	float topFrequency = baseFrequency * 11.0 /sampleRate;
	
	// adding wave from the bank
	osc->addWaveTable(sample.getSize(), sample.getData(), topFrequency, WFidX, WFidY, NOF_Y_WF);	
		
	// adding half fft wave
	topFrequency *= 2.0;
	fftOffs /= 2;
	tmp128.copyFrom(fft.subArray(0,fftOffs-1));
	fourier128->ifft(tmp128, dest128);
	osc->addWaveTable(dest128.getSize(), dest128.getData(), topFrequency, WFidX, WFidY, NOF_Y_WF);	
		
	// adding quarter fft wave 
	topFrequency *= 2.0;
	fftOffs /= 2;
	tmp64.copyFrom(fft.subArray(0,fftOffs-1));
	fourier64->ifft(tmp64, dest64);
	osc->addWaveTable(dest64.getSize(), dest64.getData(), topFrequency, WFidX, WFidY, NOF_Y_WF);
	
	// adding high end wave 	
	while (topFrequency < 14000.0/sampleRate)	{   // must be < 20Khz for full spectrum
		topFrequency *= 2.0;
		fftOffs /= 2;
		fft.setMagnitude(zeros, fftOffs, (fft.getSize())-fftOffs);
		tmp64.copyFrom(fft.subArray(0,fftOffs));
		fourier64->ifft(tmp, dest64);
		osc->addWaveTable(dest64.getSize(), dest64.getData(), topFrequency, WFidX, WFidY, NOF_Y_WF);
	}

	return topFrequency*sampleRate;
}



void WTFactory::makeMatrix(MorphOsc *osc, FloatArray bank, float baseFrequency)     {
	
	FloatArray tempsample;
	
	for (int WFidX=0 ; WFidX<NOF_X_WF ; WFidX++) 					// X and Y switchable here
	{
		for (int WFidY=0 ; WFidY<NOF_Y_WF ; WFidY++) 
		{
			tempsample = bank.subArray((WFidX*NOF_Y_WF + WFidY)*SAMPLE_LEN, SAMPLE_LEN);
			makeWaveTable(osc, tempsample, baseFrequency, WFidX, WFidY);
		} 
	}
    //return 0;
}
