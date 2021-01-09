#include "MorphOsc.h"

MorphOsc::MorphOsc(void) {										// initialisation
    phasor = 0.0;
    phaseInc = 0.0;
    phaseOfs = 0.5;
    totalWaves = 0;
    numWaveForms = 0;
    for (int idx = 0; idx < numWaveTableSlots; idx++) {    				// numWaveTableSlots = maximum arbitraire (> vrai max)
        WaveTables[idx].topFreq = 0;
        WaveTables[idx].waveTableLen = 0;
        WaveTables[idx].waveformid = 0;
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




// addWaveTable
//
// add wavetables in order of lowest frequency to highest
// topFreq is the highest frequency supported by a wavetable
// wavetables within an oscillator can be different lengths
//
// returns 0 upon success, or the number of wavetables if no more room is available
//
int MorphOsc::addWaveTable(int len, float *waveTableIn, float topFreq, int WFid, int numcycles) {   				//double topFreq
    	
    	
    this->numWaveForms = numcycles;
    	
    if (this->totalWaves < numWaveTableSlots) {
        float *waveTable = this->WaveTables[this->totalWaves].waveTable = new float[len];
      //  this->WaveTables[this->totalWaves].waveTable = waveTableIn; // new float[len];
        this->WaveTables[this->totalWaves].waveTableLen = len;
        this->WaveTables[this->totalWaves].topFreq = topFreq;
        this->WaveTables[this->totalWaves].waveformid = WFid;
        ++this->totalWaves;
        
        // fill in wave
        for (long idx = 0; idx < len; idx++) {
            waveTable[idx] = waveTableIn[idx];
        }
        
        return 0;
    }
    return this->numWaveForms;
}

//
// getOutput
//
// returns the current oscillator output
//
float MorphOsc::getOutputAtIndex(int waveTableIdx) {
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

    this->numWaveTables = (this->totalWaves) / (this->numWaveForms);

    int waveTableIdx = 0;
    waveTableIdx = ((int) (this->morphor * this->numWaveForms)) * ((this->numWaveTables));
    while (((this->phaseInc >= this->WaveTables[waveTableIdx].topFreq)) && (waveTableIdx < (this->totalWaves - 1))) {
        ++waveTableIdx;
    }
    
    float down = getOutputAtIndex(waveTableIdx);
    waveTableIdx += this->numWaveTables;
    if(waveTableIdx >= this->totalWaves)  {
		waveTableIdx -= this->totalWaves;
	}
    float up = getOutputAtIndex(waveTableIdx);


    // linear interpolation
    float temp = (this->morphor) * (this->numWaveForms); 
    int intPart = temp;
    float fracPart = temp - intPart;
    down *= (1.0 - fracPart);
    up *= fracPart;
    return down + up ; 
}
