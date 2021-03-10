#ifndef __DualMorphing2DPatch_hpp__
#define __DualMorphing2DPatch_hpp__

#include "OscSelector.h"
#include "VoltsPerOctave.h"

#define baseFrequency (20)  /* starting frequency of first table */  // c1 = 32.7 Hz

#include "morphing/wavetables/spectral64.h"

class DualMorphing2DPatch : public Patch {
	VoltsPerOctave hz;
private:
  OscSelector *morph1;
  OscSelector *morph2;
  
  SmoothFloat freqA;
  SmoothFloat amp;
  
public:
  DualMorphing2DPatch() {																																
	FloatArray bank(spectral64[0], SAMPLE_LEN*NOF_SAMPLES*NOF_OSC);
	WTFactory *wtf = new WTFactory();

	morph1 = new OscSelector();
	morph2 = new OscSelector();
	for (int i ; i<NOF_OSC ; i++)  {
	morph1->setWaveTables(wtf, bank, baseFrequency, i);
	morph2->setWaveTables(wtf, bank, baseFrequency, i);
	}

	  registerParameter(PARAMETER_A, "Frequency");
	  registerParameter(PARAMETER_B, "Amp");
	  registerParameter(PARAMETER_C, "morphX");
	  registerParameter(PARAMETER_D, "morphY");
;  }
  void processAudio(AudioBuffer &buffer) {
    
    float tune = getParameterValue(PARAMETER_A)*7.0 - 4.0;
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    hz.setTune(tune);
    float freqA = hz.getFrequency(left[0]);
    float morphC = getParameterValue(PARAMETER_C);  
    float morphD = getParameterValue(PARAMETER_D); 
    
    morph1->setMorphY(morphD);
    morph1->setFrequency(freqA/sampleRate);
    morph1->setMorphX(morphC);
    morph2->setMorphY(morphD);
    morph2->setFrequency(1.33484*freqA/sampleRate);
    morph2->setMorphX(morphC+0.04);
    amp = getParameterValue(PARAMETER_B);
    
    for(int n = 0; n<buffer.getSize(); n++){
	
	left[n] = morph1->get2DOutput() * amp * 0.25;
	right[n] = morph2->get2DOutput() * amp * 0.25;				
    morph1->updatePhases();				
    morph2->updatePhases();
		
	}
	}
};

#endif  
