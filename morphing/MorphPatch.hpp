#ifndef __MorphPatch_hpp__
#define __MorphPatch_hpp__

#include "VoltsPerOctave.h"

#include "WTFactory.h"

#include "Oscillator.h"
#include "Envelope.h"

#define PITCHBEND_RANGE 24

class MorphVoice : public SignalGenerator {
  // public Oscillator, Envelope?
private:
  MorphOsc* osc;
  AdsrEnvelope* env;
  float pb = 0.0f;
public:
  MorphVoice(MorphOsc* osc, AdsrEnvelope* env): osc(osc), env(env) {}
  void trigger(bool state){
    env->trigger(state);
  }
  void gate(bool state){
    env->gate(state);
  }
  void noteOn(uint8_t note, uint8_t velocity){
    float freq = 440.0f*exp2f((note - 69 + pb)/12.0);
    osc->setFrequency(freq);
    debugMessage("freq", freq);
    env->trigger(true);
  }
  void noteOff(){
    env->trigger(false);
    pb = 0.0f;
  }
  void pitchbend(uint16_t pitchbend){
    pb = (pitchbend/8192.0f)*PITCHBEND_RANGE;
  }
  float generate(){
    return osc->generate()*env->generate();
  }
  using SignalGenerator::generate;
  MorphOsc* getOscillator(){
    return osc;
  }
  AdsrEnvelope* getEnvelope(){
    return env;
  }
  void setEnvelope(float df){
    int di = (int)df;
    float attack, release;
    switch(di){
      // a/d
    case 0: // l/s
      attack = 1.0-df;
      release = 0.0;
      break;
    case 1: // s/s
      attack = 0.0;
      release = df-1;
      break;
    case 2: // s/l
      attack = df-2;
      release = 1.0;
      break;
    case 3: // l/l
      attack = 1.0;
      release = 1.0;
      break;
    }
    env->setAttack(attack);
    env->setRelease(release);
  }
  static MorphVoice* create(FloatArray wavetable, size_t sample_len, float lowestFreq, float sr){
    MorphOsc* osc = MorphOsc::create(wavetable, sample_len, lowestFreq, sr);
    AdsrEnvelope* env = AdsrEnvelope::create(sr);
    return new MorphVoice(osc, env);
  }
  static void destroy(MorphVoice* obj){
    MorphOsc::destroy((MorphOsc*)obj->osc);
    AdsrEnvelope::destroy((AdsrEnvelope*)obj->env);
    delete obj;
  }
};

class MorphPatch : public Patch {
private:
  MorphVoice* morphL;
  MorphVoice* morphR;
  Resource* resourceL;
  Resource* resourceR;
  int basenote = 60;
  SmoothFloat x;
  SmoothFloat y;
public:
  MorphPatch() {	
    resourceL = getResource("wavetable1");
    resourceR = getResource("wavetable2");
    // FloatArray bankL((float*)resourceL->getData(), SAMPLE_LEN*NOF_Y_WF*NOF_X_WF);
    // FloatArray bankR((float*)resourceR->getData(), SAMPLE_LEN*NOF_Y_WF*NOF_X_WF);
    FloatArray bankL = resourceL->asFloatArray();
    FloatArray bankR = resourceR->asFloatArray();
    // debugMessage("l/r", (int)resourceL->asFloatArray().getSize(), resourceR->asFloatArray().getSize());
    morphL = MorphVoice::create(bankL, SAMPLE_LEN, 20, getSampleRate());
    morphR = MorphVoice::create(bankR, SAMPLE_LEN, 20, getSampleRate());
    registerParameter(PARAMETER_A, "Frequency");
    registerParameter(PARAMETER_B, "Morph X");
    registerParameter(PARAMETER_C, "Morph Y");
    registerParameter(PARAMETER_D, "Envelope");
    registerParameter(PARAMETER_E, "Gain");
    setParameterValue(PARAMETER_A, 0.8);
    setParameterValue(PARAMETER_B, 0.5);
    setParameterValue(PARAMETER_C, 0.5);
    setParameterValue(PARAMETER_D, 0.5);
    setParameterValue(PARAMETER_E, 0.8);
  }

  ~MorphPatch(){
    MorphVoice::destroy(morphL);
    MorphVoice::destroy(morphR);
    Resource::destroy(resourceL);
    Resource::destroy(resourceR);
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
    if(value){
      switch(bid){
      case PUSHBUTTON:
      case BUTTON_A:
  	morphL->noteOn(basenote, 80);
  	morphR->noteOn(basenote, 80);
  	break;
      case BUTTON_B:
  	morphL->noteOn(basenote+3, 80);
  	morphR->noteOn(basenote+3, 80);
  	break;
      case BUTTON_C:
  	morphL->noteOn(basenote+7, 80);
  	morphR->noteOn(basenote+7, 80);
  	break;
      case BUTTON_D:
  	morphL->noteOn(basenote+12, 80);
  	morphR->noteOn(basenote+12, 80);
  	break;
      }
    }else{
      morphL->noteOff();
    }
  }

  void processMidi(MidiMessage msg){
    switch(msg.getStatus()) {
    case NOTE_OFF:
      msg.data[3] = 0;
      // deliberate fall-through
    case NOTE_ON:
      if(msg.data[3]){
	morphL->noteOn(msg.getNote(), msg.getVelocity());
	morphR->noteOn(msg.getNote(), msg.getVelocity());
      }else{
	morphL->noteOff();
	morphR->noteOff();
      }
      break;
    case PITCH_BEND_CHANGE:
      morphL->pitchbend(msg.getPitchBend());
      morphR->pitchbend(msg.getPitchBend());
      break;
    }
  }
    
  void processAudio(AudioBuffer &buffer) {
    basenote = getParameterValue(PARAMETER_A)*48+40;
    float x = getParameterValue(PARAMETER_B);  
    float y = getParameterValue(PARAMETER_C); 
    float env = getParameterValue(PARAMETER_D);
    float gain = getParameterValue(PARAMETER_E);
    morphL->setEnvelope(env*4);
    morphL->getOscillator()->setMorphY(y);
    morphL->getOscillator()->setMorphX(x);
    morphR->setEnvelope(env*4);
    morphR->getOscillator()->setMorphY(y);
    morphR->getOscillator()->setMorphX(x);
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    morphL->generate(left);
    left.multiply(gain);
    morphR->generate(right);
    right.multiply(gain);
  }
};

#endif   // __MorphPatch
