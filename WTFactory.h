
#include "MorphOsc.h"

#include "FastFourierTransform.h"
#include "Patch.h"




class WTFactory {
	
	private:

	FastFourierTransform *fourier; 
	FastFourierTransform *fourier128; 
	FastFourierTransform *fourier64; 
	
	ComplexFloatArray fft;
	ComplexFloatArray tmp;
	ComplexFloatArray tmp128;
	ComplexFloatArray tmp64;
	FloatArray dest;
	FloatArray dest128;
	FloatArray dest64;
	FloatArray zeros;
	
	public:
	
	
	
	WTFactory() {
		fourier = new FastFourierTransform();
		fourier->init(SAMPLE_LEN);
		fourier128 = new FastFourierTransform();
		fourier128->init(128);
		fourier64 = new FastFourierTransform();
		fourier64->init(64);
		fft = ComplexFloatArray::create(SAMPLE_LEN);
		tmp = ComplexFloatArray::create(SAMPLE_LEN);
		tmp128 = ComplexFloatArray::create(128);
		tmp64 = ComplexFloatArray::create(64);
		dest = FloatArray::create(SAMPLE_LEN);
		dest128 = FloatArray::create(128);
		dest64 = FloatArray::create(64);
		zeros = FloatArray::create(SAMPLE_LEN);
	}
	~WTFactory(){
	ComplexFloatArray::destroy(fft);
	ComplexFloatArray::destroy(tmp);
	ComplexFloatArray::destroy(tmp128);
	ComplexFloatArray::destroy(tmp64);
	FloatArray::destroy(dest);
	FloatArray::destroy(dest128);
	FloatArray::destroy(dest64);
	FloatArray::destroy(zeros);
}
	
	int calccycles (int WTlen, int fulllen);
	int makeWaveTable(MorphOsc *osc, FloatArray sample, float baseFrequency, int WFid);
	void makeMatrix(MorphOsc *osc, FloatArray fullsample, float baseFrequency);
	}; 
