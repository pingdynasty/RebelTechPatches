#include "message.h"
#include "Patch.h"



#ifndef Test_WaveTableOsc_h
#define Test_WaveTableOsc_h


//#define c1 (32.7) , c4 (261) 

#define sampleRate 48000

#define NOF_OSC 7
#define SAMPLE_LEN 256
#define NOF_SAMPLES 6
#define NOF_BandLimWT 7

const int numWaveTableSlots = NOF_BandLimWT*NOF_SAMPLES + 2;

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
    float morphor;
    float morphInc;
	float morphOfs;  // in order to put offset between two ext
    
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
    void setMorphOffset(float moffset);
    int addWaveTable(int len, float *waveTableIn, float topFreq, int WFid, int numcycles);
    
    float getMorphOutput();
	float getOutputAtIndex(int waveTableIdx);
};


// note: if you don't keep this in the range of 0-1, you'll need to make changes elsewhere
inline void MorphOsc::setFrequency(float pinc) {
    phaseInc = pinc;
}



// note: if you don't keep this in the range of 0-1, you'll need to make changes elsewhere
inline void MorphOsc::setPhaseOffset(float poffset) {
    phaseOfs = poffset;
}



inline void MorphOsc::updatePhase() {
    phasor += phaseInc;
    
    if (phasor >= 1.0)
        phasor -= 1.0;													
}

// note: if you don't keep this in the range of 0-1, you'll need to make changes elsewhere
inline void MorphOsc::setMorphing(float mposition) {
    morphor = mposition;
    if (morphor >= 1.0)
    morphor -= 1.0;	
}


// note: if you don't keep this in the range of 0-1, you'll need to make changes elsewhere
inline void MorphOsc::setMorphOffset(float moffset) {
    morphOfs = moffset;
}


#endif
