
#include "MorphOsc.h"

#include "FastFourierTransform.h"
#include "Patch.h"




class WTFactory {
	
	private:

	FastFourierTransform *fourier; 
	
	ComplexFloatArray fft;
	ComplexFloatArray tmp;
	FloatArray dest;
	FloatArray zeros;
	
	public:
	
	
	
	WTFactory() {
		fourier = new FastFourierTransform();
		fourier->init(SAMPLE_LEN);
		fft = ComplexFloatArray::create(SAMPLE_LEN);
		tmp = ComplexFloatArray::create(SAMPLE_LEN);
		dest = FloatArray::create(SAMPLE_LEN);
		zeros = FloatArray::create(SAMPLE_LEN);
	}
	~WTFactory(){
	ComplexFloatArray::destroy(fft);
	ComplexFloatArray::destroy(tmp);
	FloatArray::destroy(dest);
	FloatArray::destroy(zeros);
}
	
	int calccycles (int WTlen, int fulllen);
	int makeWaveTable(MorphOsc *osc, FloatArray sample, float baseFrequency, int WFid);
	void makeMatrix(MorphOsc *osc, FloatArray fullsample, float baseFrequency);
	}; 
