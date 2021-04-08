#include "MorphOsc.h"

#include "FastFourierTransform.h"




class WTFactory {
	
	private:

	FastFourierTransform *fourier; 
	
	ComplexFloatArray fft;
	ComplexFloatArray tmp;
	FloatArray dest;
	FloatArray zeros;
	
	public:
	
	
	
  WTFactory(size_t len) {
		fourier = new FastFourierTransform();
		fourier->init(len);
		fft = ComplexFloatArray::create(len);
		tmp = ComplexFloatArray::create(len);
		dest = FloatArray::create(len);
		zeros = FloatArray::create(len);
	}
	~WTFactory(){
	ComplexFloatArray::destroy(fft);
	ComplexFloatArray::destroy(tmp);
	FloatArray::destroy(dest);
	FloatArray::destroy(zeros);
	delete fourier;
	}
	
	int calccycles (int WTlen, int fulllen);
	int makeWaveTable(MorphOsc *osc, FloatArray sample, float baseFrequency, int WFidX, int WFidY);
	void makeMatrix(MorphOsc *osc, FloatArray fullsample, float baseFrequency);
private:
  float sampleRate =  48000;
}; 
