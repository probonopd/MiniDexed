/*
   MicroDexed

   MicroDexed is a port of the Dexed sound engine
   (https://github.com/asb2m10/dexed) for the Teensy-3.5/3.6/4.x with audio shield.
   Dexed ist heavily based on https://github.com/google/music-synthesizer-for-android

   (c)2018-2021 H. Wirtz <wirtz@parasitstudio.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/
#include <arm_math.h>
#include <limits.h>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include "dexed.h"
#include "synth.h"
#include "fm_core.h"
#include "exp2.h"
#include "sin.h"
#include "freqlut.h"
#include "controllers.h"
#include "PluginFx.h"
#include "porta.h"
#include "compressor.h"

Dexed::Dexed(uint8_t maxnotes, uint16_t rate)
{
  samplerate = float32_t(rate);

  Exp2::init();
  Tanh::init();
  Sin::init();

  Freqlut::init(rate);
  Lfo::init(rate);
  PitchEnv::init(rate);
  Env::init_sr(rate);
  Porta::init_sr(rate);
  fx.init(rate);

  currentNote = 0;
  resetControllers();
  controllers.masterTune = 0;
  controllers.opSwitch = 0x3f; // enable all operators
  lastKeyDown = -1;
  lfo.reset(data + 137);
  sustain = false;
  sostenuto = false;
  hold = false;
  voices = NULL;

  max_notes=maxnotes;
  if (max_notes > 0)
  {
    voices = new ProcessorVoice[max_notes]; // sizeof(ProcessorVoice) = 20
    for (uint8_t i = 0; i < max_notes; i++)
    {
      voices[i].dx7_note = new Dx7Note; // sizeof(Dx7Note) = 692
      voices[i].keydown = false;
      voices[i].sustained = false;
      voices[i].sostenuted = false;
      voices[i].held = false;
      voices[i].live = false;
      voices[i].key_pressed_timer = 0;
    }
  }
  else
    voices = NULL;

  used_notes=max_notes;
  setMonoMode(false);
  loadInitVoice();

  xrun = 0;
  render_time_max = 0;

  setVelocityScale(MIDI_VELOCITY_SCALING_OFF);
  setNoteRefreshMode(false);

  engineMsfa = new EngineMsfa;
  engineMkI = new EngineMkI;
  engineOpl = new EngineOpl;
  setEngineType(MKI);

#ifndef TEENSYDUINO
  compressor = new Compressor(samplerate);
#endif
  use_compressor = false;
}

Dexed::~Dexed()
{
  currentNote = -1;

  for (uint8_t note = 0; note < max_notes; note++)
    delete voices[note].dx7_note;
  delete[] voices;
}

void Dexed::setEngineType(uint8_t engine)
{
  panic();

  switch(engine)
  {
    case MSFA:
      controllers.core = (FmCore*)engineMsfa;
      engineType=MSFA;
      break;
    case MKI:
      controllers.core = (FmCore*)engineMkI;
      engineType=MKI;
      break;
    case OPL:
      controllers.core = (FmCore*)engineOpl;
      engineType=OPL;
      break;
    default:
      controllers.core = (FmCore*)engineMsfa;
      engineType=MSFA;
      break;
  }

  controllers.refresh();
}

uint8_t Dexed::getEngineType(void)
{
  return(engineType);
}

FmCore* Dexed::getEngineAddress(void)
{
  return(controllers.core);
}

void Dexed::setMaxNotes(uint8_t new_max_notes)
{
  panic();
  used_notes = constrain(new_max_notes, 0, max_notes);
}

void Dexed::activate(void)
{
  panic();
  controllers.refresh();
}

void Dexed::deactivate(void)
{
  panic();
}

void Dexed::getSamples(float* buffer, uint16_t n_samples)
{
  if (refreshVoice)
  {
    for (uint8_t i = 0; i < used_notes; i++)
    {
      if ( voices[i].live )
        voices[i].dx7_note->update(data, voices[i].midi_note, voices[i].velocity, voices[i].porta, &controllers);
    }
    lfo.reset(data + 137);
    refreshVoice = false;
  }

  arm_fill_f32(0.0, buffer, n_samples);

  for (uint16_t i = 0; i < n_samples; i += _N_)
  {
    AlignedBuf<int32_t, _N_> audiobuf;

    for (uint8_t j = 0; j < _N_; ++j)
    {
      audiobuf.get()[j] = 0;
    }

    int32_t lfovalue = lfo.getsample();
    int32_t lfodelay = lfo.getdelay();

    for (uint8_t note = 0; note < used_notes; note++)
    {
      if (voices[note].live)
      {
        voices[note].dx7_note->compute(audiobuf.get(), lfovalue, lfodelay, &controllers);

        for (uint8_t j = 0; j < _N_; ++j)
        {
          buffer[i + j] += signed_saturate_rshift(audiobuf.get()[j] >> 4, 24, 9) / 32768.0;
          audiobuf.get()[j] = 0;
        }
      }
    }
  }

  fx.process(buffer, n_samples); // Needed for fx.Gain()!!!

#ifndef TEENSYDUINO
  if (use_compressor == true)
    compressor->doCompression(buffer, n_samples);
#endif
}

void Dexed::getSamples(int16_t* buffer, uint16_t n_samples)
{
  float tmp[n_samples];

  getSamples(tmp, n_samples);
  arm_float_to_q15(tmp, (q15_t*)buffer, n_samples);
}

void Dexed::keydown(uint8_t pitch, uint8_t velo) {
    if (velo == 0) {
        keyup(pitch);
        return;
    }
    velo = uint8_t((float(velo)/127.0)*velocity_diff+0.5)+velocity_offset;
    pitch += data[144] - TRANSPOSE_FIX;
    int32_t previousKeyDown = lastKeyDown;
    lastKeyDown = pitch;
    int32_t porta = -1;
    if (controllers.portamento_enable_cc && previousKeyDown >= 0)
        porta = controllers.portamento_cc;
    // --- UNISON LOGIC ---
    if (pitch < kMaxMidiNotes) {
        unisonDetunedCount[pitch] = unisonVoices;
        for (uint8_t u = 0; u < unisonVoices; ++u) {
            float detuneOffset = ((float)u - (unisonVoices - 1) / 2.0f) * unisonDetune;
            float detunedPitch = (float)pitch + detuneOffset;
            unisonDetunedPitches[pitch][u] = detunedPitch;
            // Combine basePan and unison spread
            float pan = 0.5f;
            if (unisonVoices > 1) {
                float pos = (float)u / (unisonVoices - 1);
                pan = 0.5f + (pos - 0.5f) * unisonSpread;
            }
            // Add basePan offset and clamp
            pan = basePan + (pan - 0.5f);
            if (pan < 0.0f) pan = 0.0f;
            if (pan > 1.0f) pan = 1.0f;

            uint8_t note = currentNote;
            uint8_t keydown_counter = 0;
            bool found = false;
            if (!monoMode && noteRefreshMode) {
                for (uint8_t i = 0; i < used_notes; i++) {
                    if (voices[i].midi_note == detunedPitch && voices[i].keydown == false && voices[i].live &&
                       (voices[i].sustained == true || voices[i].held == true)) {
                        voices[i].dx7_note->keyup();
                        voices[i].midi_note = detunedPitch;
                        voices[i].velocity = velo;
                        voices[i].keydown = true;
                        voices[i].sustained = sustain;
                        voices[i].held = hold;
                        voices[i].live = true;
                        voices[i].dx7_note->init(data, detunedPitch, velo, detunedPitch, porta, &controllers);
                        voices[i].key_pressed_timer = millis();
                        unisonVoiceIndices[pitch][u] = i;
                        found = true;
                        goto next_unison;
                    }
                }
            }

            for (uint8_t i = 0; i <= used_notes; i++) {
                if (i == used_notes) {
                    uint32_t min_timer = 0xffffffff;

                    if (monoMode)
                        break;

                    // no free sound slot found, so use the oldest note slot
                    for (uint8_t n = 0; n < used_notes; n++) {
                        if (voices[n].key_pressed_timer < min_timer) {
                            min_timer = voices[n].key_pressed_timer;
                            note = n;
                        }
                    }
                    voices[note].keydown = false;
                    voices[note].sustained = false;
                    voices[note].sostenuted = false;
                    voices[note].held = false;
                    voices[note].live = false;
                    voices[note].key_pressed_timer = 0;
                    keydown_counter--;
                }

                if (!voices[note].keydown && !voices[note].sostenuted) {
                    currentNote = (note + 1) % used_notes;
                    lfo.keydown();
                    voices[note].midi_note = detunedPitch;
                    voices[note].velocity = velo;
                    voices[note].sustained = sustain;
                    voices[note].sostenuted = false;
                    voices[note].held = hold;
                    voices[note].keydown = true;
                    int32_t srcnote = (previousKeyDown >= 0) ? previousKeyDown : detunedPitch;
                    voices[note].dx7_note->init(data, detunedPitch, velo, srcnote, porta, &controllers);
                    if (data[136])
                      voices[note].dx7_note->oscSync();
                    voices[note].key_pressed_timer = millis();
                    keydown_counter++;
                    // --- Unison LOGIC ---
                    // set pan for each unison voice
                    voices[note].pan = pan;
                    unisonVoiceIndices[pitch][u] = note;
                    found = true;
                    break;
                } else {
                  keydown_counter++;
                }
                note = (note + 1) % used_notes;
            }

            if (!found && monoMode) {
                for (uint8_t i = 0; i < used_notes; i++) {
                    if ( voices[i].live ) {
                        // all keys are up, only transfer signal
                        if ( ! voices[i].keydown ) {
                          voices[i].live = false;
                          voices[note].dx7_note->transferSignal(*voices[i].dx7_note);
                          unisonVoiceIndices[pitch][u] = note;
                          found = true;
                          break;
                        }
                        if ( voices[i].midi_note < detunedPitch ) {
                          voices[i].live = false;
                          voices[note].dx7_note->transferState(*voices[i].dx7_note);
                          unisonVoiceIndices[pitch][u] = note;
                          found = true;
                          break;
                        }
                        goto next_unison;
                    }
                }
            }

            voices[note].live = true;
            next_unison:;
        }
    }
}

void Dexed::keyup(uint8_t pitch) {
    pitch = constrain(pitch, 0, 127);

    pitch += data[144] - TRANSPOSE_FIX;

    // Release all detuned unison voices for this note
    if (pitch < kMaxMidiNotes) {
        uint8_t count = unisonDetunedCount[pitch];
        for (uint8_t u = 0; u < count; ++u) {
            int note = unisonVoiceIndices[pitch][u];
            if (note < 0 || note >= used_notes) continue;
            if (!voices[note].keydown) continue;
            voices[note].keydown = false;
            voices[note].key_pressed_timer = 0;

            if ( monoMode )
            {
              int8_t highNote = -1;
              uint8_t target = 0;
              for (int8_t i = 0; i < used_notes; i++) {
                if ( voices[i].keydown && voices[i].midi_note > highNote ) {
                  target = i;
                  highNote = voices[i].midi_note;
                }
              }

              if ( highNote != -1 && voices[note].live ) {
                voices[note].live = false;
                voices[note].key_pressed_timer = 0;
                voices[target].live = true;
                voices[target].dx7_note->transferState(*voices[note].dx7_note);
              }
            }

            if ( voices[note].sostenuted )
              continue;

            if ( sustain ) {
              voices[note].sustained = true;
            } else if ( hold ) {
              voices[note].held = true;
            } else {
              voices[note].dx7_note->keyup();
            }
        }
        unisonDetunedCount[pitch] = 0;
    }
}

void Dexed::doRefreshVoice(void)
{
  refreshVoice = true;
}

void Dexed::setOPAll(uint8_t ops)
{
  controllers.opSwitch = ops;
}

bool Dexed::getMonoMode(void) {
  return monoMode;
}

void Dexed::setMonoMode(bool mode) {
  if (monoMode == mode)
    return;

  notesOff();
  monoMode = mode;
}

void Dexed::setNoteRefreshMode(bool mode) {
  noteRefreshMode = mode;
}

void Dexed::setSustain(bool s)
{
  if (sustain == s)
    return;

  sustain = s;

  if (!getSustain())
  {
    for (uint8_t note = 0; note < getMaxNotes(); note++)
    {
      if (voices[note].sustained && !voices[note].keydown)
      {
        voices[note].dx7_note->keyup();
        voices[note].sustained = false;
      }
    }
  }
}

bool Dexed::getSustain(void)
{
  return sustain;
}

void Dexed::setSostenuto(bool s)
{
  if (sostenuto == s)
    return;

  sostenuto = s;

  if (sostenuto)
  {
    for (uint8_t note = 0; note < getMaxNotes(); note++)
    {
      if (voices[note].keydown)
      {
        voices[note].sostenuted = true;
      }
    }
  }
  else
  {
    for (uint8_t note = 0; note < getMaxNotes(); note++)
    {
      if (voices[note].sostenuted)
      {
        voices[note].dx7_note->keyup();
        voices[note].sostenuted = false;
      }
    }
  }
}

bool Dexed::getSostenuto(void)
{
  return sostenuto;
}

void Dexed::setHold(bool h)
{
  if (hold == h)
    return;

  hold = h;

  if (!getHold())
  {
    for (uint8_t note = 0; note < getMaxNotes(); note++)
    {
      if (voices[note].held)
      {
        voices[note].dx7_note->keyup();
        voices[note].held = false;
      }
    }
  }
}

bool Dexed::getHold(void)
{
  return hold;
}

void Dexed::panic(void)
{
  for (uint8_t i = 0; i < max_notes; i++)
  {
    if (voices[i].live == true) {
      voices[i].keydown = false;
      voices[i].live = false;
      voices[i].sustained = false;
      voices[i].sostenuted = false;
      voices[i].held = false;
      voices[i].key_pressed_timer = 0;
      if ( voices[i].dx7_note != NULL ) {
        voices[i].dx7_note->oscSync();
      }
    }
  }
  setSustain(0);
}

void Dexed::resetControllers(void)
{
  controllers.values_[kControllerPitch] = 0x2000;
  controllers.values_[kControllerPitchRange] = 0;
  controllers.values_[kControllerPitchStep] = 0;
  controllers.values_[kControllerPortamentoGlissando] = 0;

  controllers.modwheel_cc = 0;
  controllers.foot_cc = 0;
  controllers.breath_cc = 0;
  controllers.aftertouch_cc = 0;
  controllers.portamento_enable_cc = false;
  controllers.portamento_cc = 0;
  controllers.refresh();
}

void Dexed::notesOff(void) {
  for (uint8_t i = 0; i < max_notes; i++) {
    if (voices[i].live == true) {
      voices[i].keydown = false;
      voices[i].live = false;
    }
  }
}

uint8_t Dexed::getMaxNotes(void)
{
  return used_notes;
}

uint8_t Dexed::getNumNotesPlaying(void)
{
  uint8_t op_carrier = controllers.core->get_carrier_operators(data[134]); // look for carriers
  uint8_t i;
  uint8_t count_playing_voices = 0;

  for (i = 0; i < used_notes; i++)
  {
    if (voices[i].live == true)
    {
      uint8_t op_amp = 0;
      uint8_t op_carrier_num = 0;

      memset(&voiceStatus, 0, sizeof(VoiceStatus));
      voices[i].dx7_note->peekVoiceStatus(voiceStatus);

      for (uint8_t op = 0; op < 6; op++)
      {
        if ((op_carrier & (1 << op)))
        {
          // this voice is a carrier!
          op_carrier_num++;
          if (voiceStatus.amp[op] <= VOICE_SILENCE_LEVEL && voiceStatus.ampStep[op] == 4)
          {
            // this voice produces no audio output
            op_amp++;
          }
        }
      }

      if (op_amp == op_carrier_num)
      {
        // all carrier-operators are silent -> disable the voice
        voices[i].live = false;
        voices[i].sustained = false;
        voices[i].sostenuted = false;
        voices[i].held = false;
        voices[i].keydown = false;
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
        Serial.print(F("Shutdown voice: "));
        Serial.println(i, DEC);
#endif
      }
      else
        count_playing_voices++;
    }
  }
  return (count_playing_voices);
}

bool Dexed::decodeVoice(uint8_t* new_data, uint8_t* encoded_data)
{
  uint8_t* p_data = new_data;
  uint8_t op;
  uint8_t tmp;
  char dexed_voice_name[11];

  panic();

  for (op = 0; op < 6; op++)
  {
    //  DEXED_OP_EG_R1,           // 0
    //  DEXED_OP_EG_R2,           // 1
    //  DEXED_OP_EG_R3,           // 2
    //  DEXED_OP_EG_R4,           // 3
    //  DEXED_OP_EG_L1,           // 4
    //  DEXED_OP_EG_L2,           // 5
    //  DEXED_OP_EG_L3,           // 6
    //  DEXED_OP_EG_L4,           // 7
    //  DEXED_OP_LEV_SCL_BRK_PT,  // 8
    //  DEXED_OP_SCL_LEFT_DEPTH,  // 9
    //  DEXED_OP_SCL_RGHT_DEPTH,  // 10
    memcpy(&new_data[op * 21], &encoded_data[op * 17], 11);
    tmp = encoded_data[(op * 17) + 11];
    *(p_data + DEXED_OP_SCL_LEFT_CURVE + (op * 21)) = (tmp & 0x03);
    *(p_data + DEXED_OP_SCL_RGHT_CURVE + (op * 21)) = (tmp & 0x0c) >> 2;
    tmp = encoded_data[(op * 17) + 12];
    *(p_data + DEXED_OP_OSC_DETUNE + (op * 21)) = (tmp & 0x78) >> 3;
    *(p_data + DEXED_OP_OSC_RATE_SCALE + (op * 21)) = (tmp & 0x07);
    tmp = encoded_data[(op * 17) + 13];
    *(p_data + DEXED_OP_KEY_VEL_SENS + (op * 21)) = (tmp & 0x1c) >> 2;
    *(p_data + DEXED_OP_AMP_MOD_SENS + (op * 21)) = (tmp & 0x03);
    *(p_data + DEXED_OP_OUTPUT_LEV + (op * 21)) = encoded_data[(op * 17) + 14];
    tmp = encoded_data[(op * 17) + 15];
    *(p_data + DEXED_OP_FREQ_COARSE + (op * 21)) = (tmp & 0x3e) >> 1;
    *(p_data + DEXED_OP_OSC_MODE + (op * 21)) = (tmp & 0x01);
    *(p_data + DEXED_OP_FREQ_FINE + (op * 21)) = encoded_data[(op * 17) + 16];
  }
  //  DEXED_PITCH_EG_R1,        // 0
  //  DEXED_PITCH_EG_R2,        // 1
  //  DEXED_PITCH_EG_R3,        // 2
  //  DEXED_PITCH_EG_R4,        // 3
  //  DEXED_PITCH_EG_L1,        // 4
  //  DEXED_PITCH_EG_L2,        // 5
  //  DEXED_PITCH_EG_L3,        // 6
  //  DEXED_PITCH_EG_L4,        // 7
  memcpy(&new_data[DEXED_VOICE_OFFSET], &encoded_data[102], 8);
  tmp = encoded_data[110];
  *(p_data + DEXED_VOICE_OFFSET + DEXED_ALGORITHM) = (tmp & 0x1f);
  tmp = encoded_data[111];
  *(p_data + DEXED_VOICE_OFFSET + DEXED_OSC_KEY_SYNC) = (tmp & 0x08) >> 3;
  *(p_data + DEXED_VOICE_OFFSET + DEXED_FEEDBACK) = (tmp & 0x07);
  //  DEXED_LFO_SPEED,          // 11
  //  DEXED_LFO_DELAY,          // 12
  //  DEXED_LFO_PITCH_MOD_DEP,  // 13
  //  DEXED_LFO_AMP_MOD_DEP,    // 14
  memcpy(&new_data[DEXED_VOICE_OFFSET + DEXED_LFO_SPEED], &encoded_data[112], 4);
  tmp = encoded_data[116];
  *(p_data + DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_SENS) = (tmp & 0x30) >> 4;
  *(p_data + DEXED_VOICE_OFFSET + DEXED_LFO_WAVE) = (tmp & 0x0e) >> 1;
  *(p_data + DEXED_VOICE_OFFSET + DEXED_LFO_SYNC) = (tmp & 0x01);
  *(p_data + DEXED_VOICE_OFFSET + DEXED_TRANSPOSE) = encoded_data[117];
  memcpy(&new_data[DEXED_VOICE_OFFSET + DEXED_NAME], &encoded_data[118], 10);
  panic();
  doRefreshVoice();

  strncpy(dexed_voice_name, (char *)&encoded_data[118], sizeof(dexed_voice_name) - 1);
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  Serial.print(F("Voice ["));
  Serial.print(dexed_voice_name);
  Serial.println(F("] decoded."));
#endif

  return (true);
}

bool Dexed::encodeVoice(uint8_t* encoded_data)
{
  uint8_t* p_data = data;
  uint8_t op;

  for (op = 0; op < 6; op++)
  {
    //  DEXED_OP_EG_R1,           // 0
    //  DEXED_OP_EG_R2,           // 1
    //  DEXED_OP_EG_R3,           // 2
    //  DEXED_OP_EG_R4,           // 3
    //  DEXED_OP_EG_L1,           // 4
    //  DEXED_OP_EG_L2,           // 5
    //  DEXED_OP_EG_L3,           // 6
    //  DEXED_OP_EG_L4,           // 7
    //  DEXED_OP_LEV_SCL_BRK_PT,  // 8
    //  DEXED_OP_SCL_LEFT_DEPTH,  // 9
    //  DEXED_OP_SCL_RGHT_DEPTH,  // 10
    memcpy(&encoded_data[op * 17], &data[op * 21], 11);
    encoded_data[(op * 17) + 11] = ((*(p_data + DEXED_OP_SCL_RGHT_CURVE + (op * 21)) & 0x0c) << 2) | (*(p_data + DEXED_OP_SCL_LEFT_CURVE + (op * 21)) & 0x03);
    encoded_data[(op * 17) + 12] = ((*(p_data + DEXED_OP_OSC_DETUNE + (op * 21)) & 0x0f) << 3) | (*(p_data + DEXED_OP_OSC_RATE_SCALE + (op * 21)) & 0x07);
    encoded_data[(op * 17) + 13] = ((*(p_data + DEXED_OP_KEY_VEL_SENS + (op * 21)) & 0x07) << 2) | (*(p_data + DEXED_OP_AMP_MOD_SENS + (op * 21)) & 0x03);
    encoded_data[(op * 17) + 14] = *(p_data + DEXED_OP_OUTPUT_LEV + (op * 21));
    encoded_data[(op * 17) + 15] = ((*(p_data + DEXED_OP_FREQ_COARSE + (op * 21)) & 0x1f) << 1) | (*(p_data + DEXED_OP_OSC_MODE + (op * 21)) & 0x01);
    encoded_data[(op * 17) + 16] = *(p_data + DEXED_OP_FREQ_FINE + (op * 21));
  }
  //  DEXED_PITCH_EG_R1,        // 0
  //  DEXED_PITCH_EG_R2,        // 1
  //  DEXED_PITCH_EG_R3,        // 2
  //  DEXED_PITCH_EG_R4,        // 3
  //  DEXED_PITCH_EG_L1,        // 4
  //  DEXED_PITCH_EG_L2,        // 5
  //  DEXED_PITCH_EG_L3,        // 6
  //  DEXED_PITCH_EG_L4,        // 7
  memcpy(&encoded_data[102], &data[DEXED_VOICE_OFFSET], 8);
  encoded_data[110] = (*(p_data + DEXED_VOICE_OFFSET + DEXED_ALGORITHM) & 0x1f);
  encoded_data[111] = (((*(p_data + DEXED_VOICE_OFFSET + DEXED_OSC_KEY_SYNC) & 0x01) << 3) | ((*(p_data + DEXED_VOICE_OFFSET + DEXED_FEEDBACK)) & 0x07));
  //  DEXED_LFO_SPEED,          // 11
  //  DEXED_LFO_DELAY,          // 12
  //  DEXED_LFO_PITCH_MOD_DEP,  // 13
  //  DEXED_LFO_AMP_MOD_DEP,    // 14
  memcpy(&encoded_data[112], &data[DEXED_VOICE_OFFSET + DEXED_LFO_SPEED], 4);
  encoded_data[116] = (((*(p_data + DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_SENS) & 0x07) << 4) | (((*(p_data + DEXED_VOICE_OFFSET + DEXED_LFO_WAVE)) & 0x07) << 1) | ((*(p_data + DEXED_VOICE_OFFSET + DEXED_LFO_SYNC)) & 0x01));
  encoded_data[117] = *(p_data + DEXED_VOICE_OFFSET + DEXED_TRANSPOSE);
  memset(&encoded_data[118], 0, 10);
  memcpy(&encoded_data[118], &data[DEXED_VOICE_OFFSET + DEXED_NAME], 10);

  return (true);
}

bool Dexed::getVoiceData(uint8_t* data_copy)
{
  memcpy(data_copy, data, sizeof(data));
  return (true);
}

void Dexed::setVoiceDataElement(uint8_t address, uint8_t value)
{
  address = constrain(address, 0, NUM_VOICE_PARAMETERS);
  data[address] = value;
  doRefreshVoice();
}

uint8_t Dexed::getVoiceDataElement(uint8_t address)
{
  address = constrain(address, 0, NUM_VOICE_PARAMETERS);
  return (data[address]);
}

void Dexed::loadVoiceParameters(uint8_t* new_data)
{
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  char dexed_voice_name[11];
#endif

  panic();
  memcpy(&data, new_data, 155);
  doRefreshVoice();
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  strncpy(dexed_voice_name, (char *)&new_data[145], sizeof(dexed_voice_name) - 1);

  Serial.print(F("Voice ["));
  Serial.print(dexed_voice_name);
  Serial.println(F("] loaded."));
#endif
}

void Dexed::loadInitVoice(void)
{
  loadVoiceParameters(init_voice);
}

void Dexed::setPBController(uint8_t pb_range, uint8_t pb_step)
{
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  Serial.println(F("Dexed::setPBController"));
#endif

  pb_range = constrain(pb_range, 0, 12);
  pb_step = constrain(pb_step, 0, 12);

  controllers.values_[kControllerPitchRange] = pb_range;
  controllers.values_[kControllerPitchStep] = pb_step;

  controllers.refresh();
}

void Dexed::setMWController(uint8_t mw_range, uint8_t mw_assign, uint8_t mw_mode)
{
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  Serial.println(F("Dexed::setMWController"));
#endif

  mw_range = constrain(mw_range, 0, 99);
  mw_assign = constrain(mw_assign, 0, 7);
  mw_mode = constrain(mw_mode, 0, MIDI_CONTROLLER_MODE_MAX);

  controllers.wheel.setRange(mw_range);
  controllers.wheel.setTarget(mw_assign);
  controllers.wheel.setMode(mw_mode);

  controllers.refresh();
}

void Dexed::setFCController(uint8_t fc_range, uint8_t fc_assign, uint8_t fc_mode)
{
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  Serial.println(F("Dexed::setFCController"));
#endif

  fc_range = constrain(fc_range, 0, 99);
  fc_assign = constrain(fc_assign, 0, 7);
  fc_mode = constrain(fc_mode, 0, MIDI_CONTROLLER_MODE_MAX);

  controllers.foot.setRange(fc_range);
  controllers.foot.setTarget(fc_assign);
  controllers.foot.setMode(fc_mode);

  controllers.refresh();
}

void Dexed::setBCController(uint8_t bc_range, uint8_t bc_assign, uint8_t bc_mode)
{
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  Serial.println(F("Dexed::setBCController"));
#endif

  bc_range = constrain(bc_range, 0, 99);
  bc_assign = constrain(bc_assign, 0, 7);
  bc_mode = constrain(bc_mode, 0, MIDI_CONTROLLER_MODE_MAX);

  controllers.breath.setRange(bc_range);
  controllers.breath.setTarget(bc_assign);
  controllers.breath.setMode(bc_mode);

  controllers.refresh();
}

void Dexed::setATController(uint8_t at_range, uint8_t at_assign, uint8_t at_mode)
{
#if defined(MICRODEXED_VERSION) && defined(DEBUG)
  Serial.println(F("Dexed::setATController"));
#endif

  at_range = constrain(at_range, 0, 99);
  at_assign = constrain(at_assign, 0, 7);
  at_mode = constrain(at_mode, 0, MIDI_CONTROLLER_MODE_MAX);

  controllers.at.setRange(at_range);
  controllers.at.setTarget(at_assign);
  controllers.at.setMode(at_mode);

  controllers.refresh();
}

void Dexed::setPortamento(uint8_t portamento_mode, uint8_t portamento_glissando, uint8_t portamento_time)
{
  portamento_mode = constrain(portamento_mode, 0, 1);
  portamento_glissando = constrain(portamento_glissando, 0, 1);
  portamento_time = constrain(portamento_time, 0, 99);

  controllers.portamento_cc = portamento_time;
  controllers.portamento_enable_cc = portamento_mode > 0;

  if (portamento_time > 0)
    controllers.portamento_enable_cc = true;
  else
    controllers.portamento_enable_cc = false;

  controllers.values_[kControllerPortamentoGlissando] = portamento_glissando;

  controllers.refresh();
}

void Dexed::setPortamentoMode(uint8_t portamento_mode)
{
  portamento_mode = constrain(portamento_mode, 0, 1);
  controllers.portamento_enable_cc = portamento_mode > 0;

  controllers.refresh();
}

uint8_t Dexed::getPortamentoMode(void)
{
  return(controllers.portamento_enable_cc);
}

void Dexed::setPortamentoGlissando(uint8_t portamento_glissando)
{
  portamento_glissando = constrain(portamento_glissando, 0, 1);
  controllers.values_[kControllerPortamentoGlissando] = portamento_glissando;

  controllers.refresh();
}

uint8_t Dexed::getPortamentoGlissando(void)
{
  return(controllers.values_[kControllerPortamentoGlissando]);
}

void Dexed::setPortamentoTime(uint8_t portamento_time)
{
  portamento_time = constrain(portamento_time, 0, 99);
  controllers.portamento_cc = portamento_time;

  if (portamento_time > 0)
    controllers.portamento_enable_cc = true;
  else
    controllers.portamento_enable_cc = false;

  controllers.refresh();
}

uint8_t Dexed::getPortamentoTime(void)
{
  return(controllers.portamento_cc);
}

int16_t Dexed::checkSystemExclusive(const uint8_t* sysex, const uint16_t len)
/*
        -1:     SysEx end status byte not detected.
        -2:     SysEx vendor not Yamaha.
        -3:     Unknown SysEx parameter change.
        -4:     Unknown SysEx voice or function.
        -5:     Not a SysEx voice bulk upload.
        -6:     Wrong length for SysEx voice bulk upload (not 155).
        -7:     Checksum error for one voice.
        -8:     Not a SysEx bank bulk upload.
        -9:     Wrong length for SysEx bank bulk upload (not 4096).
        -10:    Checksum error for bank.
        -11:    Unknown SysEx message.
	64-77:	Function parameter changed.
	100:	Voice loaded.
	200:	Bank loaded.
	300-455:	Voice parameter changed.
*/
{
  int32_t bulk_checksum_calc = 0;
  const int8_t bulk_checksum = sysex[161];

  // Check for SYSEX end byte
  if (sysex[len - 1] != 0xf7)
    return(-1);

  // check for Yamaha sysex
  if (sysex[1] != 0x43)
    return(-2);

  // Decode SYSEX by means of length
  switch (len)
  {
    case 7: // parse parameter change
      if (((sysex[3] & 0x7c) >> 2) != 0 && ((sysex[3] & 0x7c) >> 2) != 2)
        return(-3);

      if ((sysex[3] & 0x7c) >> 2 == 0) // Voice parameter
      {
        setVoiceDataElement((sysex[4] & 0x7f) + ((sysex[3] & 0x03) * 128), sysex[5]);
	doRefreshVoice();
	return((sysex[4] & 0x7f) + ((sysex[3] & 0x03) * 128)+300);
      }
      else if ((sysex[3] & 0x7c) >> 2 == 2) // Function parameter
        return(sysex[4]);
      else
	return(-4);
      break;
    case 163: // 1 Voice bulk upload
      if ((sysex[3] & 0x7f) != 0)
        return(-5);

      if (((sysex[4] << 7) | sysex[5]) != 0x9b)
        return(-6);

      // checksum calculation
      for (uint8_t i = 0; i < 155 ; i++)
        bulk_checksum_calc -= sysex[i + 6];
      bulk_checksum_calc &= 0x7f;

      if (bulk_checksum_calc != bulk_checksum)
        return(-7);

      return(100);
      break;
    case 4104: // 1 Bank bulk upload
      if ((sysex[3] & 0x7f) != 9)
        return(-8);

      if (((sysex[4] << 7) | sysex[5]) != 0x1000)
        return(-9);

      // checksum calculation
      for (uint16_t i = 0; i < 4096 ; i++)
        bulk_checksum_calc -= sysex[i + 6];
      bulk_checksum_calc &= 0x7f;

      if (bulk_checksum_calc != bulk_checksum)
        return(-10);

      return(200);
      break;
    default:
      return(-11);
  }
}

uint32_t Dexed::getXRun(void)
{
  return (xrun);
}

uint16_t Dexed::getRenderTimeMax(void)
{
  return (render_time_max);
}

void Dexed::resetRenderTimeMax(void)
{
  render_time_max = 0;
}

void Dexed::ControllersRefresh(void)
{
  controllers.refresh();
}

void Dexed::setMasterTune(int8_t mastertune)
{
  mastertune = constrain(mastertune, -99, 99);

  controllers.masterTune = (int(mastertune / 100.0 * 0x4000) << 11) * (1.0 / 12.0);
}

int8_t Dexed::getMasterTune(void)
{
  return (controllers.masterTune);
}

void Dexed::setModWheel(uint8_t value)
{
  value = constrain(value, 0, 127);

  controllers.modwheel_cc = value;
}

uint8_t Dexed::getModWheel(void)
{
  return (controllers.modwheel_cc);
}

void Dexed::setBreathController(uint8_t value)
{
  value = constrain(value, 0, 127);

  controllers.breath_cc = value;
}

uint8_t Dexed::getBreathController(void)
{
  return (controllers.breath_cc);
}

void Dexed::setFootController(uint8_t value)
{
  value = constrain(value, 0, 127);

  controllers.foot_cc = value;
}

uint8_t Dexed::getFootController(void)
{
  return (controllers.foot_cc);
}

void Dexed::setAftertouch(uint8_t value)
{
  value = constrain(value, 0, 127);

  controllers.aftertouch_cc = value;
}

uint8_t Dexed::getAftertouch(void)
{
  return (controllers.aftertouch_cc);
}

void Dexed::setPitchbend(uint8_t value1, uint8_t value2)
{
  setPitchbend(uint16_t(((value2 & 0x7f) << 7) | (value1 & 0x7f)));
}

void Dexed::setPitchbend(int16_t value)
{
  value = constrain(value, -8192, 8191);

  controllers.values_[kControllerPitch] = value + 0x2000; // -8192 to +8191 --> 0 to 16383
  setPitchbend(uint16_t(value + 0x2000)); // -8192 to +8191 --> 0 to 16383
}

void Dexed::setPitchbend(uint16_t value)
{
  controllers.values_[kControllerPitch] = (value & 0x3fff);
}

int16_t Dexed::getPitchbend(void)
{
  return (controllers.values_[kControllerPitch] - 0x2000);
}

void Dexed::setPitchbendRange(uint8_t range)
{
  range = constrain(range, 0, 12);

  controllers.values_[kControllerPitchRange] = range;
}

uint8_t Dexed::getPitchbendRange(void)
{
  return (controllers.values_[kControllerPitchRange]);
}

void Dexed::setPitchbendStep(uint8_t step)
{
  step = constrain(step, 0, 12);

  controllers.values_[kControllerPitchStep] = step;
}

uint8_t Dexed::getPitchbendStep(void)
{
  return (controllers.values_[kControllerPitchStep]);
}

void Dexed::setModWheelRange(uint8_t range)
{
  range = constrain(range, 0, 12);

  controllers.wheel.setRange(range);
}

uint8_t Dexed::getModWheelRange(void)
{
  return (controllers.wheel.getRange());
}

void Dexed::setModWheelTarget(uint8_t target)
{
  target = constrain(target, 0, 7);

  controllers.wheel.setTarget(target);
}

uint8_t Dexed::getModWheelTarget(void)
{
  return (controllers.wheel.getTarget());
}

void Dexed::setFootControllerRange(uint8_t range)
{
  range = constrain(range, 0, 12);

  controllers.foot.setRange(range);
}

uint8_t Dexed::getFootControllerRange(void)
{
  return (controllers.foot.getRange());
}

void Dexed::setFootControllerTarget(uint8_t target)
{
  target = constrain(target, 0, 7);

  controllers.foot.setTarget(target);
}

uint8_t Dexed::getFootControllerTarget(void)
{
  return (controllers.foot.getTarget());
}

void Dexed::setBreathControllerRange(uint8_t range)
{
  range = constrain(range, 0, 12);

  controllers.breath.setRange(range);
}

uint8_t Dexed::getBreathControllerRange(void)
{
  return (controllers.breath.getRange());
}

void Dexed::setBreathControllerTarget(uint8_t target)
{
  target = constrain(target, 0, 7);

  controllers.breath.setTarget(target);
}

uint8_t Dexed::getBreathControllerTarget(void)
{
  return (controllers.breath.getTarget());
}

void Dexed::setAftertouchRange(uint8_t range)
{
  range = constrain(range, 0, 12);

  controllers.at.setRange(range);
}

uint8_t Dexed::getAftertouchRange(void)
{
  return (controllers.at.getRange());
}

void Dexed::setAftertouchTarget(uint8_t target)
{
  target = constrain(target, 0, 7);

  controllers.at.setTarget(target);
}

uint8_t Dexed::getAftertouchTarget(void)
{
  return (controllers.at.getTarget());
}

void Dexed::setFilterCutoff(float cutoff)
{
  fx.Cutoff = cutoff;
}

float Dexed::getFilterCutoff(void)
{
  return (fx.Cutoff);
}

void Dexed::setFilterResonance(float resonance)
{
  fx.Reso = resonance;
}

float Dexed::getFilterResonance(void)
{
  return (fx.Reso);
}

void Dexed::setGain(float gain)
{
  fx.Gain = gain;
}

float Dexed::getGain(void)
{
  return (fx.Gain);
}

void Dexed::setOPRateAll(uint8_t rate)
{
  rate = constrain(rate, 0, 99);

  for (uint8_t op = 0; op < 6; op++)
  {
    for (uint8_t step = 0; step < 4; step++)
    {
      data[(op * 21) + DEXED_OP_EG_R1 + step] = rate;
    }
  }
  doRefreshVoice();
}

void Dexed::setOPLevelAll(uint8_t level)
{
  level = constrain(level, 0, 99);

  for (uint8_t op = 0; op < 6; op++)
  {
    for (uint8_t step = 0; step < 4; step++)
    {
      data[(op * 21) + DEXED_OP_EG_L1 + step] = level;
    }
  }
  doRefreshVoice();
}

void Dexed::setOPRateAllModulator(uint8_t step, uint8_t rate)
{
  uint8_t op_carrier = controllers.core->get_carrier_operators(data[134]); // look for carriers

  rate = constrain(rate, 0, 99);
  step = constrain(step, 0, 3);

  for (uint8_t op = 0; op < 6; op++)
  {
    if ((op_carrier & (1 << op)) == 0)
      data[(op * 21) + DEXED_OP_EG_R1 + step] = rate;
  }
  doRefreshVoice();
}

void Dexed::setOPLevelAllModulator(uint8_t step, uint8_t level)
{
  uint8_t op_carrier = controllers.core->get_carrier_operators(data[134]); // look for carriers

  step = constrain(step, 0, 3);
  level = constrain(level, 0, 99);

  for (uint8_t op = 0; op < 6; op++)
  {
    if ((op_carrier & (1 << op)) == 0)
      data[(op * 21) + DEXED_OP_EG_L1 + step] = level;
  }
  doRefreshVoice();
}

void Dexed::setOPRateAllCarrier(uint8_t step, uint8_t rate)
{
  uint8_t op_carrier = controllers.core->get_carrier_operators(data[134]); // look for carriers

  rate = constrain(rate, 0, 99);
  step = constrain(step, 0, 3);

  for (uint8_t op = 0; op < 6; op++)
  {
    if ((op_carrier & (1 << op)) == 1)
      data[(op * 21) + DEXED_OP_EG_R1 + step] = rate;
  }
  doRefreshVoice();
}

void Dexed::setOPLevelAllCarrier(uint8_t step, uint8_t level)
{
  uint8_t op_carrier = controllers.core->get_carrier_operators(data[134]); // look for carriers

  level = constrain(level, 0, 99);
  step = constrain(step, 0, 3);

  for (uint8_t op = 0; op < 6; op++)
  {
    if ((op_carrier & (1 << op)) == 1)
      data[(op * 21) + DEXED_OP_EG_L1 + step] = level;
  }
  doRefreshVoice();
}

void Dexed::setOPRate(uint8_t op, uint8_t step, uint8_t rate)
{
  op = constrain(op, 0, 5);
  step = constrain(step, 0, 3);
  rate = constrain(rate, 0, 99);

  data[(op * 21) + DEXED_OP_EG_R1 + step] = rate;
  doRefreshVoice();
}

uint8_t Dexed::getOPRate(uint8_t op, uint8_t step)
{
  op = constrain(op, 0, 5);
  step = constrain(step, 0, 3);

  return (data[(op * 21) + DEXED_OP_EG_R1 + step]);
}

void Dexed::setOPLevel(uint8_t op, uint8_t step, uint8_t level)
{
  op = constrain(op, 0, 5);
  step = constrain(step, 0, 3);
  level = constrain(level, 0, 99);

  data[(op * 21) + DEXED_OP_EG_L1 + step] = level;
  doRefreshVoice();
}

uint8_t Dexed::getOPLevel(uint8_t op, uint8_t step)
{
  op = constrain(op, 0, 5);
  step = constrain(step, 0, 3);

  return (data[(op * 21) + DEXED_OP_EG_L1 + step]);
}

void Dexed::setOPKeyboardLevelScalingBreakPoint(uint8_t op, uint8_t level)
{
  op = constrain(op, 0, 5);
  level = constrain(level, 0, 99);

  data[(op * 21) + DEXED_OP_LEV_SCL_BRK_PT] = level;
  doRefreshVoice();
}

uint8_t Dexed::getOPKeyboardLevelScalingBreakPoint(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_LEV_SCL_BRK_PT]);
}

void Dexed::setOPKeyboardLevelScalingDepthLeft(uint8_t op, uint8_t depth)
{
  op = constrain(op, 0, 5);
  depth = constrain(depth, 0, 99);

  data[(op * 21) + DEXED_OP_SCL_LEFT_DEPTH] = depth;
  doRefreshVoice();
}

uint8_t Dexed::getOPKeyboardLevelScalingDepthLeft(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_SCL_LEFT_DEPTH]);
}

void Dexed::setOPKeyboardLevelScalingDepthRight(uint8_t op, uint8_t depth)
{
  op = constrain(op, 0, 5);
  depth = constrain(depth, 0, 99);

  data[(op * 21) + DEXED_OP_SCL_RGHT_DEPTH] = depth;
  doRefreshVoice();
}

uint8_t Dexed::getOPKeyboardLevelScalingDepthRight(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_SCL_RGHT_DEPTH]);
}

void Dexed::setOPKeyboardLevelScalingCurveLeft(uint8_t op, uint8_t curve)
{
  op = constrain(op, 0, 5);
  curve = constrain(curve, 0, 3);

  data[(op * 21) + DEXED_OP_SCL_LEFT_CURVE] = curve;
  doRefreshVoice();
}

uint8_t Dexed::getOPKeyboardLevelScalingCurveLeft(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_SCL_LEFT_CURVE]);
}

void Dexed::setOPKeyboardLevelScalingCurveRight(uint8_t op, uint8_t curve)
{
  op = constrain(op, 0, 5);
  curve = constrain(curve, 0, 3);

  data[(op * 21) + DEXED_OP_SCL_RGHT_CURVE] = curve;
  doRefreshVoice();
}

uint8_t Dexed::getOPKeyboardLevelScalingCurveRight(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_SCL_RGHT_CURVE]);
}

void Dexed::setOPKeyboardRateScale(uint8_t op, uint8_t scale)
{
  op = constrain(op, 0, 5);
  scale = constrain(scale, 0, 7);

  data[(op * 21) + DEXED_OP_OSC_RATE_SCALE] = scale;
  doRefreshVoice();
}

uint8_t Dexed::getOPKeyboardRateScale(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_OSC_RATE_SCALE]);
}

void Dexed::setOPAmpModulationSensity(uint8_t op, uint8_t sensitivity)
{
  op = constrain(op, 0, 5);
  sensitivity = constrain(sensitivity, 0, 3);

  data[(op * 21) + DEXED_OP_AMP_MOD_SENS] = sensitivity;
  doRefreshVoice();
}

uint8_t Dexed::getOPAmpModulationSensity(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_AMP_MOD_SENS]);
}

void Dexed::setOPKeyboardVelocitySensity(uint8_t op, uint8_t sensitivity)
{
  op = constrain(op, 0, 5);
  sensitivity = constrain(sensitivity, 0, 7);

  data[(op * 21) + DEXED_OP_KEY_VEL_SENS] = sensitivity;
  doRefreshVoice();
}

uint8_t Dexed::getOPKeyboardVelocitySensity(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_KEY_VEL_SENS]);
}

void Dexed::setOPOutputLevel(uint8_t op, uint8_t level)
{
  op = constrain(op, 0, 5);
  level = constrain(level, 0, 99);

  data[(op * 21) + DEXED_OP_OUTPUT_LEV] = level;
  doRefreshVoice();
}

uint8_t Dexed::getOPOutputLevel(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_OUTPUT_LEV]);
}

void Dexed::setOPMode(uint8_t op, uint8_t mode)
{
  op = constrain(op, 0, 5);
  mode = constrain(mode, 0, 1);

  data[(op * 21) + DEXED_OP_OSC_MODE] = mode;
  doRefreshVoice();
}

uint8_t Dexed::getOPMode(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_OSC_MODE]);
}

void Dexed::setOPFrequencyCoarse(uint8_t op, uint8_t frq_coarse)
{
  op = constrain(op, 0, 5);
  frq_coarse = constrain(frq_coarse, 0, 31);

  data[(op * 21) + DEXED_OP_FREQ_COARSE] = frq_coarse;
  doRefreshVoice();
}

uint8_t Dexed::getOPFrequencyCoarse(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_FREQ_COARSE ]);
}

void Dexed::setOPFrequencyFine(uint8_t op, uint8_t frq_fine)
{
  op = constrain(op, 0, 5);
  frq_fine = constrain(frq_fine, 0, 99);

  data[(op * 21) + DEXED_OP_FREQ_FINE] = frq_fine;
  doRefreshVoice();
}

uint8_t Dexed::getOPFrequencyFine(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_FREQ_FINE]);
}

void Dexed::setOPDetune(uint8_t op, uint8_t detune)
{
  op = constrain(op, 0, 5);
  detune = constrain(detune, 0, 14);

  data[(op * 21) + DEXED_OP_OSC_DETUNE] = detune;
  doRefreshVoice();
}

uint8_t Dexed::getOPDetune(uint8_t op)
{
  op = constrain(op, 0, 5);

  return (data[(op * 21) + DEXED_OP_OSC_DETUNE]);
}

void Dexed::setPitchRate(uint8_t step, uint8_t rate)
{
  step = constrain(step, 0, 3);
  rate = constrain(rate, 0, 99);

  data[DEXED_VOICE_OFFSET + DEXED_PITCH_EG_R1 + step] = rate;
  doRefreshVoice();
}

uint8_t Dexed::getPitchRate(uint8_t step)
{
  step = constrain(step, 0, 3);

  return (data[DEXED_VOICE_OFFSET + DEXED_PITCH_EG_R1 + step]);
}

void Dexed::setPitchLevel(uint8_t step, uint8_t level)
{
  step = constrain(step, 0, 3);
  level = constrain(level, 0, 99);

  data[DEXED_VOICE_OFFSET + DEXED_PITCH_EG_L1 + step] = level;
  doRefreshVoice();
}

uint8_t Dexed::getPitchLevel(uint8_t step)
{
  step = constrain(step, 0, 3);

  return (data[DEXED_VOICE_OFFSET + DEXED_PITCH_EG_L1 + step]);
}

void Dexed::setAlgorithm(uint8_t algorithm)
{
  algorithm  = constrain(algorithm, 0, 31);

  data[DEXED_VOICE_OFFSET + DEXED_ALGORITHM] = algorithm;
  doRefreshVoice();
}

uint8_t Dexed::getAlgorithm(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_ALGORITHM]);
}

void Dexed::setFeedback(uint8_t feedback)
{
  feedback  = constrain(feedback, 0, 31);

  data[DEXED_VOICE_OFFSET + DEXED_FEEDBACK] = feedback;
  doRefreshVoice();
}

uint8_t Dexed::getFeedback(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_FEEDBACK]);
}

void Dexed::setOscillatorSync(bool sync)
{
  data[DEXED_VOICE_OFFSET + DEXED_OSC_KEY_SYNC] = sync;
  doRefreshVoice();
}

bool Dexed::getOscillatorSync(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_OSC_KEY_SYNC]);
}

void Dexed::setLFOSpeed(uint8_t speed)
{
  speed  = constrain(speed, 0, 99);

  data[DEXED_VOICE_OFFSET + DEXED_LFO_SPEED] = speed;
  lfo.reset(data + 137);
}

uint8_t Dexed::getLFOSpeed(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_LFO_SPEED]);
}

void Dexed::setLFODelay(uint8_t delay)
{
  delay  = constrain(delay, 0, 99);

  data[DEXED_VOICE_OFFSET + DEXED_LFO_DELAY] = delay;
  lfo.reset(data + 137);
}

uint8_t Dexed::getLFODelay(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_LFO_DELAY]);
}

void Dexed::setLFOPitchModulationDepth(uint8_t depth)
{
  depth = constrain(depth, 0, 99);

  data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_DEP] = depth;
  lfo.reset(data + 137);
}

uint8_t Dexed::getLFOPitchModulationDepth(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_DEP]);
}

void Dexed::setLFOAmpModulationDepth(uint8_t depth)
{
  depth = constrain(depth, 0, 99);

  data[DEXED_VOICE_OFFSET + DEXED_LFO_AMP_MOD_DEP] = depth;
  lfo.reset(data + 137);
}

uint8_t Dexed::getLFOAmpModulationDepth(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_LFO_AMP_MOD_DEP]);
}

void Dexed::setLFOSync(bool sync)
{
  data[DEXED_VOICE_OFFSET + DEXED_LFO_SYNC] = sync;
}

bool Dexed::getLFOSync(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_LFO_SYNC]);
  lfo.reset(data + 137);
}

void Dexed::setLFOWaveform(uint8_t waveform)
{
  waveform = constrain(waveform, 0, 5);

  data[DEXED_VOICE_OFFSET + DEXED_LFO_WAVE] = waveform;
  lfo.reset(data + 137);
}

uint8_t Dexed::getLFOWaveform(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_LFO_WAVE]);
}

void Dexed::setLFOPitchModulationSensitivity(uint8_t sensitivity)
{
  sensitivity  = constrain(sensitivity, 0, 5);

  data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_SENS] = sensitivity;
  lfo.reset(data + 137);
}

uint8_t Dexed::getLFOPitchModulationSensitivity(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_SENS]);
}

void Dexed::setTranspose(uint8_t transpose)
{
  transpose = constrain(transpose, 0, 48);

  data[DEXED_VOICE_OFFSET + DEXED_TRANSPOSE] = transpose;
}

uint8_t Dexed::getTranspose(void)
{
  return (data[DEXED_VOICE_OFFSET + DEXED_TRANSPOSE]);
}

void Dexed::setName(char name[11])
{
  strncpy((char*)&data[DEXED_VOICE_OFFSET + DEXED_NAME], name, 10);
}

void Dexed::getName(char buffer[11])
{
  strncpy(buffer, (char*)&data[DEXED_VOICE_OFFSET + DEXED_NAME], 10);
  buffer[10] = 0;
}

void Dexed::setVelocityScale(uint8_t offset, uint8_t max)
{
  velocity_offset = offset & 0x7f;
  velocity_max = max & 0x7f;
  velocity_diff = velocity_max - velocity_offset;
}

void Dexed::getVelocityScale(uint8_t* offset, uint8_t* max)
{
  *offset = velocity_offset;
  *max = velocity_max;
}

void Dexed::setVelocityScale(uint8_t setup = MIDI_VELOCITY_SCALING_OFF)
{
  switch(setup)
  {
    case MIDI_VELOCITY_SCALING_OFF:
      velocity_offset=0;
      velocity_max=127;
      break;
    case MIDI_VELOCITY_SCALING_DX7:
      velocity_offset=16;
      velocity_max=109;
      break;
    case MIDI_VELOCITY_SCALING_DX7II:
      velocity_offset=6;
      velocity_max=119;
      break;
    default:
      velocity_offset=0;
      velocity_max=127;
      break;
  }
  setVelocityScale(velocity_offset, velocity_max);
}

#ifndef TEENSYDUINO
void Dexed::setCompressor(bool enable_compressor)
{
  use_compressor = enable_compressor;
}

bool Dexed::getCompressor(void)
{
  return (use_compressor);
}

void Dexed::setCompressorPreGain_dB(float pre_gain)
{
  compressor->setPreGain_dB(pre_gain);
}

void Dexed::setCompressorAttack_sec(float attack_sec)
{
  compressor->setAttack_sec(attack_sec, samplerate);
}

void Dexed::setCompressorRelease_sec(float release_sec)
{
  compressor->setRelease_sec(release_sec, samplerate);
}

void Dexed::setCompressorThresh_dBFS(float thresh_dBFS)
{
  compressor->setThresh_dBFS(thresh_dBFS);
}

void Dexed::setCompressionRatio(float comp_ratio)
{
  compressor->setCompressionRatio(comp_ratio);
}

float Dexed::getCompressorPreGain_dB(void)
{
  return (compressor->getPreGain_dB());
}

float Dexed::getCompressorAttack_sec(void)
{
  return (compressor->getAttack_sec());
}

float Dexed::getCompressorRelease_sec(void)
{
  return (compressor->getRelease_sec());
}

float Dexed::getCompressorThresh_dBFS(void)
{
  return (compressor->getThresh_dBFS());
}

float Dexed::getCompressionRatio(void)
{
  return (compressor->getCompressionRatio());
}
#endif

#ifdef ARM_ALLOW_MULTI_CORE
void Dexed::setUnisonParameters(uint8_t voices, float detune, float spread, float basePan) {
    unisonVoices = voices;
    unisonDetune = detune;
    unisonSpread = spread;
    // Store basePan for use in keydown
    this->basePan = basePan;
}

void Dexed::getSamplesStereo(float* bufferL, float* bufferR, uint16_t n_samples) {
    if (refreshVoice) {
        for (uint8_t i = 0; i < used_notes; i++) {
            if (voices[i].live)
                voices[i].dx7_note->update(data, voices[i].midi_note, voices[i].velocity, voices[i].porta, &controllers);
        }
        lfo.reset(data + 137);
        refreshVoice = false;
    }
    arm_fill_f32(0.0, bufferL, n_samples);
    arm_fill_f32(0.0, bufferR, n_samples);
    for (uint16_t i = 0; i < n_samples; i += _N_) {
        AlignedBuf<int32_t, _N_> audiobuf;
        for (uint8_t j = 0; j < _N_; ++j) {
            audiobuf.get()[j] = 0;
        }
        int32_t lfovalue = lfo.getsample();
        int32_t lfodelay = lfo.getdelay();
        for (uint8_t note = 0; note < used_notes; note++) {
            if (voices[note].live) {
                voices[note].dx7_note->compute(audiobuf.get(), lfovalue, lfodelay, &controllers);
                float pan = voices[note].pan;
                float leftGain = cosf(pan * 1.57079633f);  // pan: 0.0=left, 1.0=right
                float rightGain = sinf(pan * 1.57079633f);
                for (uint8_t j = 0; j < _N_; ++j) {
                    float sample = signed_saturate_rshift(audiobuf.get()[j] >> 4, 24, 9) / 32768.0f;
                    bufferL[i + j] += sample * leftGain;
                    bufferR[i + j] += sample * rightGain;
                    audiobuf.get()[j] = 0;
                }
            }
        }
    }
    fx.process(bufferL, n_samples);
    fx.process(bufferR, n_samples);

#ifndef TEENSYDUINO
    if (use_compressor == true) {
        compressor->doCompression(bufferL, n_samples);
        compressor->doCompression(bufferR, n_samples);
    }
#endif
}
#endif
