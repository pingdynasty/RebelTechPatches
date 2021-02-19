////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 
 
 LICENSE:
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */


////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __VocoderClassicPatch_hpp__
#define __VocoderClassicPatch_hpp__

/*
 * Classic Vocoder Patch.
 * Controls :
 * ParameterA = Frequency of first band
 * ParameterB = Frequency of last band
 * ParameterC = Q of filter bank
 * ParameterD = 
 *
 * Inspired from Bela oriented vocoder from 
 * AlWared
 */

#include "Patch.h"
//#include "basicmath.h"
//#include "MonochromeScreenPatch.h"
//#include "MonochromeAudioDisplay.hpp"

#include "envelope_detector.h"
#include "eq.h"
#include "fb_compressor.h"
#include "vocoder.h"

/****************************************************************************************************
 * Biquad Parametric EQ filter class
 */
 
vocoder *vcd;
float *a, *b;

float mix_carrier, imix_carrier;

eq_coeffs* vox_lpf;

//int gAudioFramesPerAnalogFrame;
//int gAnalogVoxChannel = 6;     // Set ADC channel to be used for microphone

//DC removal filter parameters
typedef struct dc_remover_t {
	float fs;
	float ifs;
	
	//filter coefficient
	float a;
	
	//State Variables
	float x1, y1;
} dc_remover;

dc_remover *vox_dcr;

class VocoderClassicPatch : public Patch {
private:

public:
  VocoderClassicPatch() {   	
	float fs = getSampleRate();
	int nsamps = getBlockSize();
	
	a =  (float*) malloc(nsamps*sizeof(float));
	b =  (float*) malloc(nsamps*sizeof(float));
	
	int bands = 16;
    float fstart = 150.0;
    float fstop = 4000.0;
    vcd = make_vocoder(vcd, fs, nsamps, bands,  fstart, fstop);

		
	vox_dcr = (dc_remover*) malloc(sizeof(dc_remover));
	vox_dcr->fs = getSampleRate();
	vox_dcr->ifs = 1.0/getSampleRate();
	vox_dcr->a = expf(-vox_dcr->ifs*2.0*M_PI*150.0); //set high pass cut-off
	vox_dcr->x1 = 0.0;
	vox_dcr->y1 = 0.0;
	
	vox_lpf = make_eq_band(LPF, vox_lpf, fs, 4000.0, 0.707, 1.0);
	mix_carrier = 0.98;
	imix_carrier = 1.0 - mix_carrier;
  }    

  void processAudio(AudioBuffer &buffer){
    //float a = getFirstFreq(PARAMETER_A);
 
    float numSamples = buffer.getSize();
    float* left = buffer.getSamples(0);
    float* right = buffer.getSamples(1);

    float x0 = 0.0;
	float y0 = 0.0;
	
	for(unsigned int n = 0; n < buffer.getSize(); n++) 
	{
		//vox[n] = analogRead(context, n/gAudioFramesPerAnalogFrame, gAnalogVoxChannel);
		x0 = left[n];
		a[n] = x0;
		y0 = vox_dcr->y1 + x0 - vox_dcr->x1;
		y0 *= vox_dcr->a;
		vox_dcr->x1 = left[n];
		vox_dcr->y1 = y0;
		left[n] = tick_eq_band(vox_lpf, y0);
		b[n] = left[n];
		right[n] = imix_carrier*y0 + mix_carrier*right[n];
	}

	vocoder_tick_n(vcd, right, left);
	
	
  }

};


#endif // __VocoderClassicPatch_hpp__


////////////////////////////////////////////////////////////////////////////////////////////////////
