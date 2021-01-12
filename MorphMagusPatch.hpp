#ifndef __MorphMagusPatch_hpp__
#define __MorphMagusPatch_hpp__

#include "MonochromeScreenPatch.h"
#include "OscSelector.h"
#include "VoltsPerOctave.h"
#include "MonochromeAudioDisplay.hpp"

#define baseFrequency (20)  /* starting frequency of first table */  // c1 = 32.7 Hz

#include "alien10752.h"		/* left channel wavetable */ 
#include "additive10752.h"	/* right channel wavetable */ 

class MorphMagusPatch : public MonochromeScreenPatch {
	VoltsPerOctave hzL;
private:
  OscSelector *morphL;
  OscSelector *morphR;
  
  SmoothFloat freqL;
  float FML;
  SmoothFloat freqR;
  float FMR;
  
public:
  MorphMagusPatch() {																																
	FloatArray bankL(alien42[0], SAMPLE_LEN*NOF_SAMPLES*NOF_OSC);																																
	FloatArray bankR(additive42[0], SAMPLE_LEN*NOF_SAMPLES*NOF_OSC);
	WTFactory *wtf = new WTFactory();

	morphL = new OscSelector();
	morphR = new OscSelector();
	for (int i ; i<NOF_OSC ; i++)  {
	morphL->setWaveTables(wtf, bankL, baseFrequency, i);
	morphR->setWaveTables(wtf, bankR, baseFrequency, i);
	}
	
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
    
// Super parameters
	  registerParameter(PARAMETER_BA, "Tune");   
	  
    //setParameterValue(PARAMETER_BA, 0.5); 
;  }
  void processAudio(AudioBuffer &buffer) {
    
    float npast;
    float FML = getParameterValue(PARAMETER_B);
    float FMR = getParameterValue(PARAMETER_D);
    float tuneL = getParameterValue(PARAMETER_A)-0.5;
    float tune = getParameterValue(PARAMETER_BA)-0.5;
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    hzL.setTune(tune/12.0);
    float freq = hzL.getFrequency(left[0]);
    float freqL = exp(log(freq)+log(2)*tuneL/12.0);
    float morphXL = getParameterValue(PARAMETER_AA);  
    float morphYL = getParameterValue(PARAMETER_AB); 
    
	float offsetR = getParameterValue(PARAMETER_C)*24;
	//int offsetR = getParameterValue(PARAMETER_C)*24;
    float freqR = exp(log(freq)+log(2)*offsetR/12.0);
    float morphXR = getParameterValue(PARAMETER_AC);  
    float morphYR = getParameterValue(PARAMETER_AD); 
    
    
    morphL->setMorphY(morphYL);					// always set Y before X
    morphL->setFrequency(freqL/sampleRate);
    morphL->setMorphX(morphXL);
    morphR->setMorphY(morphYR);
    morphR->setFrequency((freqR)/sampleRate);
    morphR->setMorphX(morphXR);

    
    for(int n = 0; n<buffer.getSize(); n++){
	
	left[n] = (morphL->get2DOutput()) * 0.5;
	right[n] = morphR->get2DOutput() * 0.5;	
    morphL->updatePhases();				
    morphR->updatePhases();
    //leftright[n] = left[n];
    //leftright[n+buffer.getSize()] = right[n];
		
	}    
	
    display.update(left, 2, 0.0, 3.0, 0.0);
    debugMessage("out" , freq , FMR )	;	

	}
	
MonochromeAudioDisplay display;

  void processScreen(MonochromeScreenBuffer& screen){
    display.draw(screen, WHITE);
   }
};

#endif   // __MorphMagusPatch
