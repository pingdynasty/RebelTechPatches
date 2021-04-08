#include "WTFactory.h"
#include "MorphOsc.h"

MorphOsc::MorphOsc(void) {										
    phasor = 0.0;
    phaseInc = 0.0;
    phaseOfs = 0.5;
    totalWaves = 0;
    numYaxisWaveForms = 0;
    numXaxisWaveForms = NOF_X_WF;
    numBLWaveForms = 0;
    for (int idx = 0; idx < numWaveTableSlots; idx++) {    				// numWaveTableSlots = maximum arbitraire (> vrai max)
        WaveTables[idx].topFreq = 0;
        WaveTables[idx].waveTableLen = 0;
        WaveTables[idx].waveformidX = 0;
        WaveTables[idx].waveformidY = 0;
        WaveTables[idx].waveTable = 0;
    }
}


MorphOsc::~MorphOsc(void) {										
    for (int idx = 0; idx < numWaveTableSlots; idx++) {
        float *temp = WaveTables[idx].waveTable;
        if (temp != 0)
            delete [] temp;
    }
}


//void MorphOsc::setWaveTables(WTFactory *wtf, FloatArray banks, float baseFrequency)  { 
		////FloatArray bank = banks.subArray(Idx*SAMPLE_LEN*NOF_Y_WF, SAMPLE_LEN*NOF_Y_WF);
		//wtf->makeMatrix(bank, baseFrequency);
    
//}


// addWaveTable
//
// add wavetables in order of lowest frequency to highest
// topFreq is the highest frequency supported by a wavetable
// wavetables within an oscillator can be different lengths
//
int MorphOsc::addWaveTable(int len, float *waveTableIn, float topFreq, int WFidX, int WFidY, int numYWF) {   				//double topFreq
    	
    	
    this->numYaxisWaveForms = numYWF;
    	
    if (this->totalWaves < numWaveTableSlots) {
        float *waveTable = this->WaveTables[this->totalWaves].waveTable = new float[len];
      //  this->WaveTables[this->totalWaves].waveTable = waveTableIn; // new float[len];
        this->WaveTables[this->totalWaves].waveTableLen = len;
        this->WaveTables[this->totalWaves].topFreq = topFreq;
        this->WaveTables[this->totalWaves].waveformidX = WFidX;
        this->WaveTables[this->totalWaves].waveformidY = WFidY;
        ++this->totalWaves;
        
        // fill in wave
        for (long idx = 0; idx < len; idx++) {
            waveTable[idx] = waveTableIn[idx];
        }
        
	this->numBLWaveForms = (this->totalWaves) / ((this->numYaxisWaveForms)*(this->numXaxisWaveForms));
        return 0;
    }

    return this->totalWaves;
}

//
// getOutputAtIndex
//
// returns the current waveform oscillator output
//
float MorphOsc::getOutputAtIndex(int waveTableIdx) {
  ASSERT(waveTableIdx < this->totalWaves, "wave table index out of bounds");
    waveTable *waveTable = &this->WaveTables[waveTableIdx];
    


    float temp = this->phasor * waveTable->waveTableLen;
    int intPart = temp;
    float fracPart = temp - intPart;
    
    float samp0 = waveTable->waveTable[intPart];
    if (++intPart >= waveTable->waveTableLen)
        intPart = waveTable->waveTableLen-1;
    float samp1 = waveTable->waveTable[intPart];
    return samp0 + (samp1 - samp0) * fracPart;
    

}

float MorphOsc::getMorphOutput() {
    // grab the appropriate extWF and then BL

    int waveTableIdx = 0;
    int temPos = 0;
    temPos = ((int) (this->morphSelX * this->numXaxisWaveForms)) * this->numYaxisWaveForms * this->numBLWaveForms; 	// go to position of low frequency first Y axis WF
    waveTableIdx = temPos + (((int) (this->morphSelY * this->numYaxisWaveForms)) * this->numBLWaveForms);							// go to position of low frequency wanted WF
    while (((this->phaseInc >= this->WaveTables[waveTableIdx].topFreq)) && (waveTableIdx < (this->totalWaves - 1))) {
        ++waveTableIdx;
    }
    
    float downXdownY = getOutputAtIndex(waveTableIdx);
    waveTableIdx += this->numBLWaveForms;
    if(waveTableIdx >= temPos + this->numYaxisWaveForms * this->numBLWaveForms)  {
		waveTableIdx -= (this->numYaxisWaveForms * this->numBLWaveForms);		// mirroring first to last Y axis WF
	}
    float downXupY = getOutputAtIndex(waveTableIdx);
    waveTableIdx += this->numYaxisWaveForms * this->numBLWaveForms;
    if(waveTableIdx >= this->numXaxisWaveForms * this->numYaxisWaveForms * this->numBLWaveForms)  {
		waveTableIdx -= this->numXaxisWaveForms * this->numYaxisWaveForms * this->numBLWaveForms;		// mirroring first to last X axis WF
	}
    float upXupY = getOutputAtIndex(waveTableIdx);
    waveTableIdx -= this->numBLWaveForms;
    if(waveTableIdx < 0)  {
		waveTableIdx += this->numYaxisWaveForms * this->numBLWaveForms;		// mirroring first to last Y axis WF
	}
    float upXdownY = getOutputAtIndex(waveTableIdx);


    // linear interpolation
    float temp = (this->morphSelY) * (this->numYaxisWaveForms); 
    int intPart = temp;
    float fracPart = temp - intPart;
    downXdownY *= (1.0 - fracPart);
    upXdownY *= (1.0 - fracPart);
    downXupY *= fracPart;
    upXupY *= fracPart;
    temp = (this->morphSelX) * (this->numXaxisWaveForms); 
    intPart = temp;
    fracPart = temp - intPart;
    //(downXdownY + downXupY) *= (1.0 - fracPart);
    //(upXdownY + upXupY) *= fracPart;
    return ((downXdownY + downXupY) * (1.0 - fracPart) + (upXdownY + upXupY) * fracPart)/2 ;
}


MorphOsc* MorphOsc::create(FloatArray wavetable, size_t sample_len, float lowestFreq, float sr){
  WTFactory* wtf = new WTFactory(sample_len);
  MorphOsc* osc = new MorphOsc();
  wtf->makeMatrix(osc, wavetable, lowestFreq);
  delete wtf;
  return osc;
}

void MorphOsc::destroy(MorphOsc* obj){
  delete obj;
}
