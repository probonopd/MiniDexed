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

//#define DISABLE_DEXED_COMPRESSOR 1

#ifndef DEXED_H_INCLUDED
#define DEXED_H_INCLUDED

#include <stdint.h>
#include <cstdlib>
#include <arm_math.h>
#if defined(TEENSYDUINO)
#include <Audio.h>
#endif
#include "fm_op_kernel.h"
#include "synth.h"
#include "env.h"
#include "aligned_buf.h"
#include "pitchenv.h"
#include "controllers.h"
#include "dx7note.h"
#include "lfo.h"
#include "PluginFx.h"
#include "compressor.h"
#include "EngineMsfa.h"
#include "EngineMkI.h"
#include "EngineOpl.h"

#define NUM_VOICE_PARAMETERS 156

struct ProcessorVoice {
  uint8_t midi_note;
  uint8_t velocity;
  int16_t porta;
  bool keydown;
  bool sustained;
  bool sostenuted;
  bool held;
  bool live;
  uint32_t key_pressed_timer;
  Dx7Note *dx7_note;
  float pan; // pan value for stereo spread (0.0 = left, 1.0 = right)
};

enum DexedVoiceOPParameters {
  DEXED_OP_EG_R1,           // 0
  DEXED_OP_EG_R2,           // 1
  DEXED_OP_EG_R3,           // 2
  DEXED_OP_EG_R4,           // 3
  DEXED_OP_EG_L1,           // 4
  DEXED_OP_EG_L2,           // 5
  DEXED_OP_EG_L3,           // 6
  DEXED_OP_EG_L4,           // 7
  DEXED_OP_LEV_SCL_BRK_PT,  // 8
  DEXED_OP_SCL_LEFT_DEPTH,  // 9
  DEXED_OP_SCL_RGHT_DEPTH,  // 10
  DEXED_OP_SCL_LEFT_CURVE,  // 11
  DEXED_OP_SCL_RGHT_CURVE,  // 12
  DEXED_OP_OSC_RATE_SCALE,  // 13
  DEXED_OP_AMP_MOD_SENS,    // 14
  DEXED_OP_KEY_VEL_SENS,    // 15
  DEXED_OP_OUTPUT_LEV,      // 16
  DEXED_OP_OSC_MODE,        // 17
  DEXED_OP_FREQ_COARSE,     // 18
  DEXED_OP_FREQ_FINE,       // 19
  DEXED_OP_OSC_DETUNE       // 20
};

#define DEXED_VOICE_OFFSET 126
enum DexedVoiceParameters {
  DEXED_PITCH_EG_R1,        // 0
  DEXED_PITCH_EG_R2,        // 1
  DEXED_PITCH_EG_R3,        // 2
  DEXED_PITCH_EG_R4,        // 3
  DEXED_PITCH_EG_L1,        // 4
  DEXED_PITCH_EG_L2,        // 5
  DEXED_PITCH_EG_L3,        // 6
  DEXED_PITCH_EG_L4,        // 7
  DEXED_ALGORITHM,          // 8
  DEXED_FEEDBACK,           // 9
  DEXED_OSC_KEY_SYNC,       // 10
  DEXED_LFO_SPEED,          // 11
  DEXED_LFO_DELAY,          // 12
  DEXED_LFO_PITCH_MOD_DEP,  // 13
  DEXED_LFO_AMP_MOD_DEP,    // 14
  DEXED_LFO_SYNC,           // 15
  DEXED_LFO_WAVE,           // 16
  DEXED_LFO_PITCH_MOD_SENS, // 17
  DEXED_TRANSPOSE,          // 18
  DEXED_NAME                // 19
};

enum ADSR {
  ATTACK,
  DECAY,
  SUSTAIN,
  RELEASE
};

enum OPERATORS {
  OP1,
  OP2,
  OP3,
  OP4,
  OP5,
  OP6
};

enum CONTROLLER_ASSIGN {
  NONE,
  PITCH,
  AMP,
  PITCH_AMP,
  EG,
  PITCH_EG,
  AMP_EG,
  PITCH_AMP_EG
};

enum PORTAMENTO_MODE {
  RETAIN,
  FOLLOW
};

enum ON_OFF {
  OFF,
  ON
};

enum VELOCITY_SCALES {
  MIDI_VELOCITY_SCALING_OFF,
  MIDI_VELOCITY_SCALING_DX7,
  MIDI_VELOCITY_SCALING_DX7II
};

enum ENGINES {
  MSFA,
  MKI,
  OPL
};

// GLOBALS

//==============================================================================

class Dexed
{
  public:
    Dexed(uint8_t maxnotes, uint16_t rate);
    ~Dexed();

    // Global methods
    void activate(void);
    void deactivate(void);
    bool getMonoMode(void);
    void setMonoMode(bool mode);
    void setNoteRefreshMode(bool mode);
    uint8_t getMaxNotes(void);
    void doRefreshVoice(void);
    void setOPAll(uint8_t ops);
    bool decodeVoice(uint8_t* data, uint8_t* encoded_data);
    bool encodeVoice(uint8_t* encoded_data);
    bool getVoiceData(uint8_t* data_copy);
    void setVoiceDataElement(uint8_t address, uint8_t value);
    uint8_t getVoiceDataElement(uint8_t address);
    void loadInitVoice(void);
    void loadVoiceParameters(uint8_t* data);
    uint8_t getNumNotesPlaying(void);
    uint32_t getXRun(void);
    uint16_t getRenderTimeMax(void);
    void resetRenderTimeMax(void);
    void ControllersRefresh(void);
    void setVelocityScale(uint8_t offset, uint8_t max);
    void getVelocityScale(uint8_t* offset, uint8_t* max);
    void setVelocityScale(uint8_t setup);
    void setMaxNotes(uint8_t n);
    void setEngineType(uint8_t engine);
    uint8_t getEngineType(void);
    FmCore* getEngineAddress(void);
#ifndef TEENSYDUINO
    void setCompressor(bool comp);
    bool getCompressor(void);
    void setCompressorPreGain_dB(float pre_gain);
    void setCompressorAttack_sec(float attack_sec);
    void setCompressorRelease_sec(float release_sec);
    void setCompressorThresh_dBFS(float thresh_dBFS);
    void setCompressionRatio(float comp_ratio);
    float getCompressorPreGain_dB(void);
    float getCompressorAttack_sec(void);
    float getCompressorRelease_sec(void);
    float getCompressorThresh_dBFS(void);
    float getCompressionRatio(void);
#endif
    int16_t checkSystemExclusive(const uint8_t* sysex, const uint16_t len);

    // Sound methods
    void keyup(uint8_t pitch);
    void keydown(uint8_t pitch, uint8_t velo);
    void setSustain(bool sustain);
    bool getSustain(void);
    void setSostenuto(bool sostenuto);
    bool getSostenuto(void);
    void setHold(bool hold);
    bool getHold(void);
    void panic(void);
    void notesOff(void);
    void resetControllers(void);
    void setMasterTune(int8_t mastertune);
    int8_t getMasterTune(void);
    void setPortamento(uint8_t portamento_mode, uint8_t portamento_glissando, uint8_t portamento_time);
    void setPortamentoMode(uint8_t portamento_mode);
    uint8_t getPortamentoMode(void);
    void setPortamentoGlissando(uint8_t portamento_glissando);
    uint8_t getPortamentoGlissando(void);
    void setPortamentoTime(uint8_t portamento_time);
    uint8_t getPortamentoTime(void);
    void setPBController(uint8_t pb_range, uint8_t pb_step);
    void setMWController(uint8_t mw_range, uint8_t mw_assign, uint8_t mw_mode);
    void setFCController(uint8_t fc_range, uint8_t fc_assign, uint8_t fc_mode);
    void setBCController(uint8_t bc_range, uint8_t bc_assign, uint8_t bc_mode);
    void setATController(uint8_t at_range, uint8_t at_assign, uint8_t at_mode);
    void setModWheel(uint8_t value);
    uint8_t getModWheel(void);
    void setBreathController(uint8_t value);
    uint8_t getBreathController(void);
    void setFootController(uint8_t value);
    uint8_t getFootController(void);
    void setAftertouch(uint8_t value);
    uint8_t getAftertouch(void);
    void setPitchbend(uint8_t value1, uint8_t value2);
    void setPitchbend(int16_t value);
    void setPitchbend(uint16_t value);
    int16_t getPitchbend(void);
    void setPitchbendRange(uint8_t range);
    uint8_t getPitchbendRange(void);
    void setPitchbendStep(uint8_t step);
    uint8_t getPitchbendStep(void);
    void setModWheelRange(uint8_t range);
    uint8_t getModWheelRange(void);
    void setModWheelTarget(uint8_t target);
    uint8_t getModWheelTarget(void);
    void setFootControllerRange(uint8_t range);
    uint8_t getFootControllerRange(void);
    void setFootControllerTarget(uint8_t target);
    uint8_t getFootControllerTarget(void);
    void setBreathControllerRange(uint8_t range);
    uint8_t getBreathControllerRange(void);
    void setBreathControllerTarget(uint8_t target);
    uint8_t getBreathControllerTarget(void);
    void setAftertouchRange(uint8_t range);
    uint8_t getAftertouchRange(void);
    void setAftertouchTarget(uint8_t target);
    uint8_t getAftertouchTarget(void);
    void setFilterCutoff(float cutoff);
    float getFilterCutoff(void);
    void setFilterResonance(float resonance);
    float getFilterResonance(void);
    void setGain(float gain);
    float getGain(void);

    // Voice configuration methods
    void setOPRateAll(uint8_t rate);
    void setOPLevelAll(uint8_t level);
    void setOPRateAllCarrier(uint8_t step, uint8_t rate);
    void setOPLevelAllCarrier(uint8_t step, uint8_t level);
    void setOPRateAllModulator(uint8_t step, uint8_t rate);
    void setOPLevelAllModulator(uint8_t step, uint8_t level);
    void setOPRate(uint8_t op, uint8_t step, uint8_t rate);
    uint8_t getOPRate(uint8_t op, uint8_t step);
    void setOPLevel(uint8_t op, uint8_t step, uint8_t level);
    uint8_t getOPLevel(uint8_t op, uint8_t step);
    void setOPKeyboardLevelScalingBreakPoint(uint8_t op, uint8_t level);
    uint8_t getOPKeyboardLevelScalingBreakPoint(uint8_t op);
    void setOPKeyboardLevelScalingDepthLeft(uint8_t op, uint8_t depth);
    uint8_t getOPKeyboardLevelScalingDepthLeft(uint8_t op);
    void setOPKeyboardLevelScalingDepthRight(uint8_t op, uint8_t depth);
    uint8_t getOPKeyboardLevelScalingDepthRight(uint8_t op);
    void setOPKeyboardLevelScalingCurveLeft(uint8_t op, uint8_t curve);
    uint8_t getOPKeyboardLevelScalingCurveLeft(uint8_t op);
    void setOPKeyboardLevelScalingCurveRight(uint8_t op, uint8_t curve);
    uint8_t getOPKeyboardLevelScalingCurveRight(uint8_t op);
    void setOPKeyboardRateScale(uint8_t op, uint8_t scale);
    uint8_t getOPKeyboardRateScale(uint8_t op);
    void setOPAmpModulationSensity(uint8_t op, uint8_t sensitivity);
    uint8_t getOPAmpModulationSensity(uint8_t op);
    void setOPKeyboardVelocitySensity(uint8_t op, uint8_t sensitivity);
    uint8_t getOPKeyboardVelocitySensity(uint8_t op);
    void setOPOutputLevel(uint8_t op, uint8_t level);
    uint8_t getOPOutputLevel(uint8_t op);
    void setOPMode(uint8_t op, uint8_t mode);
    uint8_t getOPMode(uint8_t op);
    void setOPFrequencyCoarse(uint8_t op, uint8_t frq_coarse);
    uint8_t getOPFrequencyCoarse(uint8_t op);
    void setOPFrequencyFine(uint8_t op, uint8_t frq_fine);
    uint8_t getOPFrequencyFine(uint8_t op);
    void setOPDetune(uint8_t op, uint8_t detune);
    uint8_t getOPDetune(uint8_t op);
    void setPitchRate(uint8_t step, uint8_t rate);
    uint8_t getPitchRate(uint8_t step);
    void setPitchLevel(uint8_t step, uint8_t level);
    uint8_t getPitchLevel(uint8_t step);
    void setAlgorithm(uint8_t algorithm);
    uint8_t getAlgorithm(void);
    void setFeedback(uint8_t feedback);
    uint8_t getFeedback(void);
    void setOscillatorSync(bool sync);
    bool getOscillatorSync(void);
    void setLFOSpeed(uint8_t speed);
    uint8_t getLFOSpeed(void);
    void setLFODelay(uint8_t delay);
    uint8_t getLFODelay(void);
    void setLFOPitchModulationDepth(uint8_t depth);
    uint8_t getLFOPitchModulationDepth(void);
    void setLFOAmpModulationDepth(uint8_t delay);
    uint8_t getLFOAmpModulationDepth(void);
    void setLFOSync(bool sync);
    bool getLFOSync(void);
    void setLFOWaveform(uint8_t waveform);
    uint8_t getLFOWaveform(void);
    void setLFOPitchModulationSensitivity(uint8_t sensitivity);
    uint8_t getLFOPitchModulationSensitivity(void);
    void setTranspose(uint8_t transpose);
    uint8_t getTranspose(void);
    void setName(char name[11]);
    void getName(char buffer[11]);

    ProcessorVoice* voices;

  protected:
    uint8_t init_voice[NUM_VOICE_PARAMETERS] = {
      99, 99, 99, 99, 99, 99, 99, 00, 33, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, // OP6 eg_rate_1-4, level_1-4, kbd_lev_scl_brk_pt, kbd_lev_scl_lft_depth, kbd_lev_scl_rht_depth, kbd_lev_scl_lft_curve, kbd_lev_scl_rht_curve, kbd_rate_scaling, amp_mod_sensitivity, key_vel_sensitivity, operator_output_level, osc_mode, osc_freq_coarse, osc_freq_fine, osc_detune
      99, 99, 99, 99, 99, 99, 99, 00, 33, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, // OP5
      99, 99, 99, 99, 99, 99, 99, 00, 33, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, // OP4
      99, 99, 99, 99, 99, 99, 99, 00, 33, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, // OP4
      99, 99, 99, 99, 99, 99, 99, 00, 33, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, // OP4
      99, 99, 99, 99, 99, 99, 99, 00, 33, 00, 00, 00, 00, 00, 00, 00, 99, 00, 01, 00, 00, // OP4
      99, 99, 99, 99, 50, 50, 50, 50,                                                     // 4 * pitch EG rates, 4 * pitch EG level
      01, 00, 01,                                                                         // algorithm, feedback, osc sync
      35, 00, 00, 00, 01, 00,                                                             // lfo speed, lfo delay, lfo pitch_mod_depth, lfo_amp_mod_depth, lfo_sync, lfo_waveform
      03, 48,                                                                             // pitch_mod_sensitivity, transpose
      73, 78, 73, 84, 32, 86, 79, 73, 67, 69                                              // 10 * char for name ("INIT VOICE")
    };
    float samplerate;
    uint8_t data[NUM_VOICE_PARAMETERS];
    uint8_t max_notes;
    uint8_t used_notes;
    PluginFx fx;
    Controllers controllers;
    int32_t lastKeyDown;
    uint32_t xrun;
    uint16_t render_time_max;
    int16_t currentNote;
    bool sustain;
    bool sostenuto;
    bool hold;
    bool monoMode;
    bool noteRefreshMode;
    bool refreshVoice;
    uint8_t engineType;
    VoiceStatus voiceStatus;
    Lfo lfo;
    EngineMsfa* engineMsfa;
    EngineMkI* engineMkI;
    EngineOpl* engineOpl;
    void getSamples(float* buffer, uint16_t n_samples);
    void getSamples(int16_t* buffer, uint16_t n_samples);
    void compress(float* wav_in, float* wav_out, uint16_t n, float threshold, float slope, uint16_t sr,  float tla, float twnd, float tatt, float trel);
    bool use_compressor;
    uint8_t velocity_offset;
    uint8_t velocity_max;
    float velocity_diff;
#ifndef TEENSYDUINO
    Compressor* compressor;
#endif
    // Unison state
    uint8_t unisonVoices = 1;
    float unisonDetune = 0.0f;
    float unisonSpread = 0.0f;
#ifdef ARM_ALLOW_MULTI_CORE
    // Unison parameters and stereo output only for multicore
    void setUnisonParameters(uint8_t voices, float detune, float spread, float basePan);
    void getSamplesStereo(float* bufferL, float* bufferR, uint16_t n_samples);
    float basePan = 0.5f;
#endif
    // --- Unison note tracking for robust note-off ---
    static constexpr int kMaxMidiNotes = 128;
    static constexpr int kMaxUnison = 8; // adjust as needed
    float unisonDetunedPitches[kMaxMidiNotes][kMaxUnison] = {};
    uint8_t unisonDetunedCount[kMaxMidiNotes] = {};
    int unisonVoiceIndices[kMaxMidiNotes][kMaxUnison] = {};
};

#endif
