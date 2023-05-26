/* From https://github.com/chipaudette/OpenAudio_ArduinoLibrary */

/*
   AudioEffectCompressor

   Created: Chip Audette, Dec 2016 - Jan 2017
   Purpose; Apply dynamic range compression to the audio stream.
            Assumes floating-point data.

   This processes a single stream fo audio data (ie, it is mono)

   MIT License.  use at your own risk.
*/

#ifndef _COMPRESSOR_H
#define _COMPRESSOR_H

#include <arm_math.h> //ARM DSP extensions.  https://www.keil.com/pack/doc/CMSIS/DSP/html/index.html
#include "synth.h"

class Compressor
{
  public:
    //constructor
    Compressor(const float32_t sample_rate_Hz);

    void doCompression(float32_t *audio_block, uint16_t len);
    void setDefaultValues(const float32_t sample_rate_Hz);
    void setPreGain(float32_t g);
    void setPreGain_dB(float32_t gain_dB);
    void setCompressionRatio(float32_t cr);
    void setAttack_sec(float32_t a, float32_t fs_Hz);
    void setRelease_sec(float32_t r, float32_t fs_Hz);
    void setLevelTimeConst_sec(float32_t t_sec, float32_t fs_Hz);
    void setThresh_dBFS(float32_t val);
    void enableHPFilter(boolean flag);
    float32_t getPreGain_dB(void);
    float32_t getAttack_sec(void);
    float32_t getRelease_sec(void);
    float32_t getLevelTimeConst_sec(void);
    float32_t getThresh_dBFS(void);
    float32_t getCompressionRatio(void);
    float32_t getCurrentLevel_dBFS(void);
    float32_t getCurrentGain_dB(void);

  protected:
    void calcAudioLevel_dB(float32_t *wav_block, float32_t *level_dB_block, uint16_t len);
    void calcGain(float32_t *audio_level_dB_block, float32_t *gain_block,uint16_t len);
    void calcInstantaneousTargetGain(float32_t *audio_level_dB_block, float32_t *inst_targ_gain_dB_block, uint16_t len);
    void calcSmoothedGain_dB(float32_t *inst_targ_gain_dB_block, float32_t *gain_dB_block, uint16_t len);
    void resetStates(void);
    void setHPFilterCoeff_N2IIR_Matlab(float32_t b[], float32_t a[]);
    
    //state-related variables
    float32_t *inputQueueArray_f32[1]; //memory pointer for the input to this module
    float32_t prev_level_lp_pow = 1.0;
    float32_t prev_gain_dB = 0.0; //last gain^2 used

    //HP filter state-related variables
    arm_biquad_casd_df1_inst_f32 hp_filt_struct;
    static const uint8_t hp_nstages = 1;
    float32_t hp_coeff[5 * hp_nstages] = {1.0, 0.0, 0.0, 0.0, 0.0}; //no filtering. actual filter coeff set later
    float32_t hp_state[4 * hp_nstages];
    void setHPFilterCoeff(void);
    //private parameters related to gain calculation
    float32_t attack_const, release_const, level_lp_const; //used in calcGain().  set by setAttack_sec() and setRelease_sec();
    float32_t comp_ratio_const, thresh_pow_FS_wCR;  //used in calcGain();  set in updateThresholdAndCompRatioConstants()
    void updateThresholdAndCompRatioConstants(void);
    //settings
    float32_t attack_sec, release_sec, level_lp_sec; 
    float32_t thresh_dBFS = 0.0;  //threshold for compression, relative to digital full scale
    float32_t thresh_pow_FS = 1.0f;  //same as above, but not in dB
    void setThreshPow(float32_t t_pow);
    float32_t comp_ratio = 1.0;  //compression ratio
    float32_t pre_gain = -1.0;  //gain to apply before the compression.  negative value disables
    boolean use_HP_prefilter;
};

// Accelerate the powf(10.0,x) function
static float32_t pow10f(float32_t x);
// Accelerate the log10f(x)  function?
static float32_t log10f_approx(float32_t x);
static float32_t log2f_approx(float32_t X);
#endif
