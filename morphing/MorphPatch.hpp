#ifndef __MorphPatch_hpp__
#define __MorphPatch_hpp__

#include "WTFactory.h"
#include "Envelope.h"
#include "ShortArray.h"
#include "Oscillator.h"
#include "SineOscillator.h"
#include "TapTempo.hpp"
#include "WavFile.h"
#include "AbstractSynth.h"

static const int TRIGGER_LIMIT = (1<<17);

class MorphVoice : public AbstractSynth {
private:
  MorphOsc* osc;
  AdsrEnvelope* env;
  float gain;
public:
  MorphVoice(MorphOsc* osc, AdsrEnvelope* env): osc(osc), env(env) {}  
  void setFrequency(float freq){
    osc->setFrequency(freq);    
  }
  void setGain(float gain){
    this->gain = gain;
  }
  void trigger(){
    env->trigger();
  }
  void gate(bool state){
    env->gate(state);
  }
  float generate(){
    return osc->generate()*env->generate()*gain;
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
  MonophonicProcessor<MorphVoice>* morphL;
  MonophonicProcessor<MorphVoice>* morphR;
  int basenote = 60;
  SmoothFloat x;
  SmoothFloat y;
  SmoothFloat mod;

  SineOscillator* lfo1;
  SineOscillator* lfo2;
  TapTempo<TRIGGER_LIMIT> tempo1;
  TapTempo<TRIGGER_LIMIT> tempo2;

  MorphVoice* createVoice(const char* name){
    Resource* resource = getResource(name);
    WavFile wav = WavFile::create(resource->getData());
    if(!wav.isValid())
      error(CONFIGURATION_ERROR_STATUS, "Invalid wav");
    FloatArray bank = wav.createFloatArray();
    Resource::destroy(resource);
    MorphVoice* voice = MorphVoice::create(bank, SAMPLE_LEN, 20, getSampleRate());
    FloatArray::destroy(bank);
    return voice;
  }
public:
  MorphPatch() :
    tempo1(getSampleRate()*0.5), tempo2(getSampleRate()*0.25) {

    morphL = new MonophonicProcessor<MorphVoice>(createVoice("wavetable1.wav"));
    morphR = new MonophonicProcessor<MorphVoice>(createVoice("wavetable2.wav"));
    
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

    registerParameter(PARAMETER_AA, "Decay");
    registerParameter(PARAMETER_AB, "Sustain");
    setParameterValue(PARAMETER_AA, 0.0);
    setParameterValue(PARAMETER_AB, 0.9);

    // lfo
    lfo1 = SineOscillator::create(getBlockRate());
    lfo2 = SineOscillator::create(getBlockRate());
    registerParameter(PARAMETER_F, "LFO1>");
    registerParameter(PARAMETER_G, "LFO2>");
  }

  ~MorphPatch(){
    SineOscillator::destroy(lfo1);
    SineOscillator::destroy(lfo2);
    MorphVoice::destroy(morphL->getVoice());
    MorphVoice::destroy(morphR->getVoice());
    delete morphL;
    delete morphR;
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
    int note = 0;
    switch(bid){
    case PUSHBUTTON:
    case BUTTON_A:
      note = basenote;
      break;
    case BUTTON_B:
      note = basenote+3;
      break;
    case BUTTON_C:
      tempo1.trigger(value, samples);
      if(value)
	lfo1->reset();
      // note = basenote+7;
      break;
    case BUTTON_D:
      tempo2.trigger(value, samples);
      if(value)
	lfo2->reset(); // todo: instead of hard reset, calculate to sync on next edge
      // note = basenote+12;
      break;
    }
    if(note){
      if(value){
	morphL->noteOn(MidiMessage::note(0, note, 80));
	morphR->noteOn(MidiMessage::note(0, note, 80));
      }else{
	morphL->noteOff(MidiMessage::note(0, note, 0));
	morphR->noteOff(MidiMessage::note(0, note, 0));
      }
    }
  }

  void processMidi(MidiMessage msg){
    morphL->process(msg);
    morphR->process(msg);
    if(msg.isControlChange() && msg.getControllerNumber() == 1)
      mod = msg.getControllerValue()/256.0f;
      
    // switch(msg.getStatus()) {
    // case NOTE_OFF:
    //   msg.data[3] = 0;
    //   // deliberate fall-through
    // case NOTE_ON:
    //   if(msg.getVelocity()){
    // 	morphL->noteOn(msg.getNote(), msg.getVelocity());
    // 	morphR->noteOn(msg.getNote(), msg.getVelocity());
    //   }else{
    // 	morphL->noteOff(msg.getNote(), msg.getVelocity());
    // 	morphR->noteOff(msg.getNote(), msg.getVelocity());
    //   }
    //   break;
    // case PITCH_BEND_CHANGE:
    //   morphL->pitchbend(msg.getPitchBend());
    //   morphR->pitchbend(msg.getPitchBend());
    //   break;
    // case CONTROL_CHANGE:
    //   uint8_t cc = msg.getControllerNumber();
    //   if(cc == 1){
    // 	mod = msg.getControllerValue()/256.0f;
    // 	// morphL->modulate(mod);
    // 	// morphR->modulate(mod);
    //   }
    //   break;
    // }
  }
    
  void processAudio(AudioBuffer &buffer) {
    basenote = getParameterValue(PARAMETER_A)*48+40;
    x = getParameterValue(PARAMETER_B);  
    y = getParameterValue(PARAMETER_C); 
    float env = getParameterValue(PARAMETER_D);
    float gain = getParameterValue(PARAMETER_E);
    morphL->getVoice()->setEnvelope(env*4);
    morphL->getVoice()->getOscillator()->setMorphY(y);
    morphL->getVoice()->getOscillator()->setMorphX(x+mod);
    morphR->getVoice()->setEnvelope(env*4);
    morphR->getVoice()->getOscillator()->setMorphY(y);
    morphR->getVoice()->getOscillator()->setMorphX(x+mod);
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    morphL->generate(left);
    left.multiply(gain);
    morphR->generate(right);
    right.multiply(gain);

    float decay = getParameterValue(PARAMETER_AA);
    float sustain = getParameterValue(PARAMETER_AB);
    morphL->getVoice()->getEnvelope()->setDecay(decay);
    morphL->getVoice()->getEnvelope()->setSustain(sustain);
    morphR->getVoice()->getEnvelope()->setDecay(decay);
    morphR->getVoice()->getEnvelope()->setSustain(sustain);
    
    // lfo
    tempo1.clock(getBlockSize());
    tempo2.clock(getBlockSize());
    float rate = getSampleRate()/tempo1.getSamples();
    lfo1->setFrequency(rate);
    setParameterValue(PARAMETER_F, lfo1->generate()*0.5+0.5);
    setButton(BUTTON_E, lfo1->getPhase() < M_PI);
    rate = getSampleRate()/tempo2.getSamples();
    lfo2->setFrequency(rate);
    setParameterValue(PARAMETER_G, lfo2->generate()*0.5+0.5);
    setButton(BUTTON_F, lfo2->getPhase() < M_PI);

    // debugMessage("l1/l2", lfo1->getFrequency(), lfo2->getFrequency());
  }
};

#endif   // __MorphPatch
