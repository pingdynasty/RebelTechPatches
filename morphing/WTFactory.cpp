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
    zeros.clear();
    

	dest.copyFrom(sample);
	
	fourier->fft(dest, fft);
	int fftoffs = fft.getSize();
	
	int ret = 0;
	

	float topFreq = baseFrequency * 16.0 /sampleRate;
	
	

	for (int i=0; i<(NOF_BandLimWF); i++)  {
		fftoffs /= 2;
		fft.setMagnitude(zeros, fftoffs, (fft.getSize())-fftoffs);
		tmp.copyFrom(fft);
		fourier->ifft(tmp, dest);
		ret = osc->addWaveTable(dest.getSize(), dest.getData(), topFreq, WFidX, WFidY, NOF_Y_WF);
		topFreq *= 2.0;
		
	}
	
	return ret;
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
