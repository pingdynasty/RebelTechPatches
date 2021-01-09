#include "MorphOsc.h"
#include "WTFactory.h"



//#define baseFrequency (20)  /* starting frequency of first table */  // c1 = 32.7 Hz

class OscSelector {
private:
	MorphOsc *oscs[NOF_OSC];
	MorphOsc* down;
	MorphOsc* up;
	float mix;
    
public:

    OscSelector(void);
    ~OscSelector(void);
	
	
    void setWaveTables(WTFactory *wtf, FloatArray banks, float baseFrequency, int Idx);
    
    void setMorphY(float morphY);
    void setMorphX(float morphX);
	void setFrequency(float freq);   
    float get2DOutput();
    void updatePhases();
};
