#include "message.h"
#include "Patch.h"



#ifndef Test_WaveTableOsc_h
#define Test_WaveTableOsc_h


//#define c1 (32.7) , c4 (261) 

#define sampleRate 48000

#define SAMPLE_LEN 256
//#define NOF_OSC 8
#define NOF_X_WF 8
//#define NOF_SAMPLES 8
#define NOF_Y_WF 8
#define NOF_BandLimWT 7

const int numWaveTableSlots = NOF_BandLimWT*NOF_Y_WF + 2;

typedef struct {
    float topFreq;
    int waveTableLen;
    int waveformid;
    float *waveTable;
} waveTable;

class MorphOsc {
private:
    float phasor;      // phase accumulator
    float phaseInc;    // phase increment
    float phaseOfs;    // phase offset for PWM
    float morphSel;
    float morphInc;
	//float morphOfs;  // in order to put offset between two ext
    
    // list of wavetables
public:
    int totalWaves;
    waveTable WaveTables[numWaveTableSlots];
    int numWaveForms;
    int numWaveTables;

    MorphOsc(void);
    ~MorphOsc(void);
    void setFrequency(float pinc);
    void setPhaseOffset(float poffset);
    void updatePhase(void);
	void setMorphing(float minc);
    //void setMorphOffset(float moffset);
    int addWaveTable(int len, float *waveTableIn, float topFreq, int WFid, int numcycles);
    
    float getMorphOutput();
	float getOutputAtIndex(int waveTableIdx);
	
    int getInferiorIndex();
};



inline void MorphOsc::setFrequency(float freq) {
    
    if (freq <= 10)						
        {freq = 10;}	
        
    phaseInc = freq/sampleRate;
    
    if (phaseInc >= 1.0)						
        {phaseInc = 1.0;}	

}



inline void MorphOsc::setPhaseOffset(float poffset) {
    phaseOfs = poffset;
        
    if (phaseOfs >= 1.0)
        phaseOfs -= 1.0;	
}



inline void MorphOsc::updatePhase() {
    phasor += phaseInc;
    
    if (phasor >= 1.0)
        phasor -= 1.0;													
}


inline void MorphOsc::setMorphing(float mposition) {
    morphSel = mposition;
    if (morphSel >= 1.0)
    morphSel -= 1.0;	
}


//inline void MorphOsc::setMorphOffset(float moffset) {
    //morphOfs = moffset;
//}

inline int MorphOsc::getInferiorIndex() {			// only for debuging purpose
	int waveTableIdx = 0;
    waveTableIdx = ((int) (this->morphSel * this->numWaveForms)) * ((this->numWaveTables));
    while (((this->phaseInc >= this->WaveTables[waveTableIdx].topFreq)) && (waveTableIdx < (this->totalWaves - 1))) {
        ++waveTableIdx;
    }
    return waveTableIdx;
}

#endif
