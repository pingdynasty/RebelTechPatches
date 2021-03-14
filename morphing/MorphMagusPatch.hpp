#ifndef __MorphMagusPatch_hpp__
#define __MorphMagusPatch_hpp__

#include "MonochromeScreenPatch.h"
#include "WTFactory.h"
//#include "MorphOsc.h"
#include "VoltsPerOctave.h"
#include "MonochromeAudioDisplay.hpp"

#define baseFrequency (20)  /* starting frequency of first table */  // c1 = 32.7 Hz

#include "morphing/wavetables/spectral64.h"		/* left channel wavetable */ 
#include "morphing/wavetables/mikeS64.h"	/* right channel wavetable */ 

class MorphMagusPatch : public MonochromeScreenPatch {
	VoltsPerOctave hzL;
private:
  MorphOsc *morphL;
  MorphOsc *morphR;
  
  SmoothFloat freqL;
  float FML;
  SmoothFloat freqR;
  float FMR;
  
public:
  MorphMagusPatch() {																																
	FloatArray bankL(spectral64[0], SAMPLE_LEN*NOF_Y_WF*NOF_X_WF);																																
	//FloatArray bankR(mikeS64[0], SAMPLE_LEN*NOF_Y_WF*NOF_X_WF);
	WTFactory *wtf = new WTFactory();

	morphL = new MorphOsc();
	morphR = new MorphOsc();
	wtf->makeMatrix(morphL, bankL, baseFrequency);
	//morphL->setWaveTables(wtf, bankL, baseFrequency);	
	//bankL.destroy(bankL);																															
	//FloatArray bankR(mikeS64[0], SAMPLE_LEN*NOF_Y_WF*NOF_X_WF);
	wtf->makeMatrix(morphR, bankL, baseFrequency);				// has to take bankR for L/R different wavetables
	
	
// Morhp oscilator Left
	  registerParameter(PARAMETER_A, "TuneL");
	  registerParameter(PARAMETER_B, "FML");
	  registerParameter(PARAMETER_AA, "MorphXL");
	  registerParameter(PARAMETER_AB, "MorphYL");

    setParameterValue(PARAMETER_A, 0.5);
    setParameterValue(PARAMETER_B, 0.1);
    //setParameterValue(PARAMETER_AA, 0.126);
    //setParameterValue(PARAMETER_AB, 0.6);
	  
// Morhp oscilator Right
	  registerParameter(PARAMETER_C, "OffsetR");
	  registerParameter(PARAMETER_D, "FMR");
	  registerParameter(PARAMETER_AC, "MorphXR");
	  registerParameter(PARAMETER_AD, "MorphYR");

    setParameterValue(PARAMETER_C, 0);
    setParameterValue(PARAMETER_D, 0.1);
    //setParameterValue(PARAMETER_AC, 0.126);
    //setParameterValue(PARAMETER_AD, 0.6);
    
// Modulations
    registerParameter(PARAMETER_E, "LFO1>");
    registerParameter(PARAMETER_F, "LFO2>");
    
    setParameterValue(PARAMETER_E, 0.26);
    setParameterValue(PARAMETER_F, 0.51);    
    //setParameterValue(PARAMETER_AE, 0.26);
    //setParameterValue(PARAMETER_AF, 0.51);
    
// Super parameters
	registerParameter(PARAMETER_BA, "Tune"); 
    registerParameter(PARAMETER_BB, "Tempo");
    registerParameter(PARAMETER_BC, "LFO1");
    registerParameter(PARAMETER_BD, "LFO2");
  
    setParameterValue(PARAMETER_BA, 0.5); 
    setParameterValue(PARAMETER_BB, 0.5);
    setParameterValue(PARAMETER_BC, 0.2);
    setParameterValue(PARAMETER_BD, 0.4);
  
;  }
~MorphMagusPatch(){
  }
  void processAudio(AudioBuffer &buffer) {
    
    float gain = 0.1;
    float npast;
    float FML = getParameterValue(PARAMETER_B);
    float FMR = getParameterValue(PARAMETER_D);
    float tuneL = getParameterValue(PARAMETER_A)-0.5;
    float tune = getParameterValue(PARAMETER_BA)-0.5;
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    hzL.setTune((tune/12.0)-4.0);
    float freq = hzL.getFrequency(left[0]);
    float freqL = exp(log(freq)+log(2)*tuneL/12.0);
    float morphXL = getParameterValue(PARAMETER_AA);  
    float morphYL = getParameterValue(PARAMETER_AB); 
    
	//float offsetR = getParameterValue(PARAMETER_C)*24;
	float offsetR = getParameterValue(PARAMETER_C)*24*3;
    float freqR = exp(log(freq)+log(2)*offsetR/12.0);
    float morphXR = getParameterValue(PARAMETER_AC);  
    float morphYR = getParameterValue(PARAMETER_AD); 
    
    
    morphL->setMorphY(morphYL);					// always set Y before X
    for(int n = 0; n<buffer.getSize(); n++){
    morphL->setFrequency(freqL+right[n]/gain*FML*freqL/2);
	}
    morphL->setMorphX(morphXL);
    morphR->setMorphY(morphYR);
    for(int n = 0; n<buffer.getSize(); n++){
    morphR->setFrequency(freqR+right[n]/gain*FMR*freqR/2);
	}
    morphR->setMorphX(morphXR);
    
    float tempo = getParameterValue(PARAMETER_BB)*16 + 0.5;
    
    static float lfo1 = 0;
    if(lfo1 > 1.0){
      lfo1 = 0;
    }else{
    }
    lfo1 += 2 * getBlockSize() / getSampleRate();
    tempo = getParameterValue(PARAMETER_BC)*4+0.01;
    lfo1 += tempo * getBlockSize() / getSampleRate();
    setParameterValue(PARAMETER_E, lfo1*0.4);

    static float lfo2 = 0;
    if(lfo2 > 1.0){
      lfo2 = 0;
    }else{
    }
    lfo2 += 1 * getBlockSize() / getSampleRate();
    tempo = getParameterValue(PARAMETER_BD)*4+0.01;
    lfo2 += tempo * getBlockSize() / getSampleRate();
    setParameterValue(PARAMETER_F, lfo2*0.4);
    
    for(int n = 0; n<buffer.getSize(); n++){
	
	left[n] = (morphL->getMorphOutput()) * gain;
	right[n] = morphR->getMorphOutput() * gain;	
    morphL->updatePhase();				
    morphR->updatePhase();
		
	}    
	
    display.update(left, 2, 0.0, 3.0, 0.0);
    debugMessage("out" , (int)(freqR), morphR->getInferiorIndex() )	;	

	}
	
MonochromeAudioDisplay display;

  void processScreen(MonochromeScreenBuffer& screen){
    display.draw(screen, WHITE);
   }
};

#endif   // __MorphMagusPatch
