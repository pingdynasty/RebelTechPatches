#ifndef __MorphPatch_hpp__
#define __MorphPatch_hpp__

#include "WTFactory.h"
#include "Envelope.h"
#include "ShortArray.h"
#include "Oscillator.h"
#include "SineOscillator.h"
#include "TapTempo.hpp"
static const int TRIGGER_LIMIT = (1<<17);

#define PITCHBEND_RANGE 2

typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    char     fmtchunk_id[4];
    uint32_t fmtchunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bps;
    char     datachunk_id[4];
    uint32_t datachunk_size;
}WavHeader;

class MorphVoice : public SignalGenerator {
  // public Oscillator, Envelope?
private:
  MorphOsc* osc;
  AdsrEnvelope* env;
  float pb = 0.0f;
  uint8_t note = 60;
public:
  MorphVoice(MorphOsc* osc, AdsrEnvelope* env): osc(osc), env(env) {}
  void trigger(bool state){
    env->trigger(state);
  }
  void gate(bool state){
    env->gate(state);
  }
  void setFrequency(uint8_t note, float pitchbend){
    float freq = 440.0f*exp2f((note - 69 + pitchbend)/12.0);
    osc->setFrequency(freq);    
  }
  void noteOn(uint8_t note, uint8_t velocity){
    this->note = note;
    setFrequency(note, pb);
    env->gate(true);
  }
  void noteOff(){
    env->gate(false);
  }
  void play(uint8_t note){
    this->note = note;
    setFrequency(note, pb);
    env->trigger(true);
  }
  void pitchbend(int16_t pitchbend){
    pb = (pitchbend/8192.0f)*PITCHBEND_RANGE;
    float freq = 440.0f*exp2f((note - 69 + pb)/12.0);
    osc->setFrequency(freq);
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
  SmoothFloat mod;

  SineOscillator* lfo1;
  SineOscillator* lfo2;
  TapTempo<TRIGGER_LIMIT> tempo1;
  TapTempo<TRIGGER_LIMIT> tempo2;

  ShortArray readwav(uint8_t* data){
    WavHeader* wav_header = (WavHeader*)data;
    if(strncmp(wav_header->chunk_id, "RIFF", 4) ||
       strncmp(wav_header->format, "WAVE", 4) ||
       wav_header->audio_format != 1 ||
       wav_header->datachunk_size == 0 ||
       wav_header->fmtchunk_size != 16)
      error(CONFIGURATION_ERROR_STATUS, "Invalid wav");
    const int len = wav_header->datachunk_size/(wav_header->fmtchunk_size/8);
    data += sizeof(WavHeader);
    return ShortArray((int16_t*)data, len);
  }
public:
  MorphPatch() :
    tempo1(getSampleRate()*0.5), tempo2(getSampleRate()*0.25) {
    resourceL = getResource("wavetable1.wav");
    resourceR = getResource("wavetable2.wav");

    ShortArray samples = readwav((uint8_t*)resourceL->getData());
    FloatArray bankL = FloatArray::create(samples.getSize());
    samples.toFloat(bankL);
    samples = readwav((uint8_t*)resourceR->getData());
    FloatArray bankR = FloatArray::create(samples.getSize());
    samples.toFloat(bankR);

    debugMessage("l/r", (int)bankL.getSize(), bankR.getSize());

    // FloatArray bankL = resourceL->asFloatArray();
    // FloatArray bankR = resourceR->asFloatArray();
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

    // lfo
    lfo1 = SineOscillator::create(getSampleRate());
    lfo2 = SineOscillator::create(getSampleRate());
    registerParameter(PARAMETER_F, "LFO1>");
    registerParameter(PARAMETER_G, "LFO2>");
  }

  ~MorphPatch(){
    SineOscillator::destroy(lfo1);
    SineOscillator::destroy(lfo2);
    MorphVoice::destroy(morphL);
    MorphVoice::destroy(morphR);
    Resource::destroy(resourceL);
    Resource::destroy(resourceR);
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
      note = basenote+7;
      break;
    case BUTTON_D:
      tempo2.trigger(value, samples);
      if(value)
	lfo2->reset();
      note = basenote+12;
      break;
    }
    if(note && value){
      morphL->play(note);
      morphR->play(note);
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
    case CONTROL_CHANGE:
      uint8_t cc = msg.getControllerNumber();
      if(cc == 1){
	mod = msg.getControllerValue()/256.0f;
      }
      break;
    }
  }
    
  void processAudio(AudioBuffer &buffer) {
    basenote = getParameterValue(PARAMETER_A)*48+40;
    x = getParameterValue(PARAMETER_B);  
    y = getParameterValue(PARAMETER_C); 
    float env = getParameterValue(PARAMETER_D);
    float gain = getParameterValue(PARAMETER_E);
    morphL->setEnvelope(env*4);
    morphL->getOscillator()->setMorphY(y);
    morphL->getOscillator()->setMorphX(x+mod);
    morphR->setEnvelope(env*4);
    morphR->getOscillator()->setMorphY(y);
    morphR->getOscillator()->setMorphX(x+mod);
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    morphL->generate(left);
    left.multiply(gain);
    morphR->generate(right);
    right.multiply(gain);

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

    debugMessage("l1/l2", lfo1->getFrequency(), lfo2->getFrequency());
  }
};

#endif   // __MorphPatch
