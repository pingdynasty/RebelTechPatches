#ifndef __DualMorphing2DPatch_hpp__
#define __DualMorphing2DPatch_hpp__

#include "WTFactory.h"
//#include "MorphOsc.h"
#include "VoltsPerOctave.h"

#define baseFrequency (20)  /* starting frequency of first table */  // c1 = 32.7 Hz

#include "morphing/wavetables/spectral64.h"

class DualMorphing2DPatch : public Patch {
	VoltsPerOctave hz;
private:
  MorphOsc *morph1;
  MorphOsc *morph2;
  
  SmoothFloat freqA;
  SmoothFloat amp;
  
public:
  DualMorphing2DPatch() {																																
	FloatArray bank(spectral64[0], SAMPLE_LEN*NOF_Y_WF*NOF_X_WF);
	WTFactory *wtf = new WTFactory();

	morph1 = new MorphOsc();
	morph2 = new MorphOsc();
	
	wtf->makeMatrix(morph1, bank, baseFrequency);
	wtf->makeMatrix(morph2, bank, baseFrequency);

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
    morph1->setFrequency(freqA);
    morph1->setMorphX(morphC);
    morph2->setMorphY(morphD);
    morph2->setFrequency(1.33484*freqA);
    morph2->setMorphX(morphC+0.04);
    amp = getParameterValue(PARAMETER_B);
    
    for(int n = 0; n<buffer.getSize(); n++){
	
	left[n] = morph1->getMorphOutput() * amp * 0.25;
	right[n] = morph2->getMorphOutput() * amp * 0.25;				
    morph1->updatePhase();				
    morph2->updatePhase();
		
	}
	}
};

#endif  
