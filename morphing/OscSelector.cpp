#include "OscSelector.h"

OscSelector::OscSelector(void) {										
   for (int i = 0 ; i<NOF_X_WF ; i++)  {
		oscs[i] = new MorphOsc();
	}	
	down = oscs[0];
	up = oscs[0];
	mix = 0;
}


OscSelector::~OscSelector(void) {
}



void OscSelector::setWaveTables(WTFactory *wtf, FloatArray banks, float baseFrequency, int Idx)  { 
		FloatArray bank = banks.subArray(Idx*SAMPLE_LEN*NOF_Y_WF, SAMPLE_LEN*NOF_Y_WF);
		wtf->makeMatrix(oscs[Idx], bank, baseFrequency);
    
}

void OscSelector::setMorphY(float morphY){
	float temp = morphY * NOF_X_WF;
    int intPart = temp;
    mix = temp - intPart;
    
    down = oscs[intPart];
    intPart++;
	if (intPart >= NOF_X_WF) {
        intPart = 0 ;
	}
	up = oscs[intPart];
}

//
// getOutput
//
// returns 2 oscillators outputs
//
float OscSelector::get2DOutput() {
   
    float downValue = down -> getMorphOutput() * (1 - mix);

    float upValue = up -> getMorphOutput() * (mix);
    return downValue + upValue;

}


void OscSelector::setFrequency(float freq)  {
   
   down -> setFrequency(freq);
   up -> setFrequency(freq);  
    
}

void OscSelector::setMorphX(float morphX)  {
   
   down -> setMorphing(morphX);
   up -> setMorphing(morphX);   
    
}

void OscSelector::updatePhases()  {
	
	down -> updatePhase();
    up -> updatePhase();
    
}

int OscSelector::getInferiorIndex() {
    return down -> getInferiorIndex();
}
