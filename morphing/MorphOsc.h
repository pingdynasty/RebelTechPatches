#include "basicmaths.h"
#include "Oscillator.h"

#ifndef Test_WaveTableOsc_h
#define Test_WaveTableOsc_h


//#define c1 (32.7) , c4 (261) 

#define SAMPLE_LEN 256
//#define NOF_OSC 8
#define NOF_X_WF 8
//#define NOF_SAMPLES 8
#define NOF_Y_WF 8
#define NOF_BandLimWF 7

const int numWaveTableSlots = NOF_BandLimWF*NOF_Y_WF*NOF_X_WF;

typedef struct {
    float topFreq;
    int waveTableLen;
    int waveformidX;
    int waveformidY;
    float *waveTable;
} waveTable;

class MorphOsc : public Oscillator {
private:
    float phasor;      // phase accumulator
    float phaseInc;    // phase increment
    float phaseOfs;    // phase offset for PWM
    float morphSelX;
    float morphSelY;
    float morphInc;
	//float morphOfs;  // in order to put offset between two ext
    
    // list of wavetables
public:
    int totalWaves;
    waveTable WaveTables[numWaveTableSlots];
    int numYaxisWaveForms;
    int numXaxisWaveForms;
    int numBLWaveForms;

    MorphOsc(void);
    ~MorphOsc(void);
    void setFrequency(float pinc);
    void setPhaseOffset(float poffset);
  void setPhase(float phase){
    phasor = phase/(2*M_PI);
  }
  float getPhase(){
    return phasor*2*M_PI;
  }
    void updatePhase(void);
	void setMorphX(float mPos);
	void setMorphY(float mPos);
    //void setMorphOffset(float moffset);	
    //void setWaveTables(WTFactory *wtf, FloatArray banks, float baseFrequency);
    int addWaveTable(int len, float *waveTableIn, float topFreq, int WFidX, int WFidY, int numcycles); // numcycles mandatory here ?
    
    float getMorphOutput();
	float getOutputAtIndex(int waveTableIdx);
	
    int getInferiorIndex();
    float generate() {
      float sample = getMorphOutput();    
      updatePhase();
      return sample;
    }
  float generate(float fm){
    float sample = getMorphOutput();
    phasor += fm;
    updatePhase();
    return sample;
  }
  static MorphOsc* create(FloatArray wavetable, size_t sample_len, float lowestFreq, float sr);
  static void destroy(MorphOsc* obj);
private:
  float sampleRate =  48000;

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


inline void MorphOsc::setMorphX(float positionX) {
    morphSelX = positionX;
    if (morphSelX >= 1.0)
    morphSelX -= 1.0;	
}


inline void MorphOsc::setMorphY(float positionY) {
    morphSelY = positionY;
    if (morphSelY >= 1.0)
    morphSelY -= 1.0;	
}


//inline void MorphOsc::setMorphOffset(float moffset) {
    //morphOfs = moffset;
//}

inline int MorphOsc::getInferiorIndex() {			// only for debuging purpose
	int waveTableIdx = 0;
    int temPos = 0;
    temPos = ((int) (this->morphSelX * this->numXaxisWaveForms)) * this->numYaxisWaveForms * this->numBLWaveForms; 	// go to position of low frequency first Y axis WF
    waveTableIdx = temPos + (((int) (this->morphSelY * this->numYaxisWaveForms)) * this->numBLWaveForms);							// go to position of low frequency wanted WF
    while (((this->phaseInc >= this->WaveTables[waveTableIdx].topFreq)) && (waveTableIdx < (this->totalWaves - 1))) {
        ++waveTableIdx;
    }
    return waveTableIdx;
}

#endif
