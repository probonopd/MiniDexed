/* From https://github.com/chipaudette/OpenAudio_ArduinoLibrary */

/*
   AudioEffectCompressor

   Created: Chip Audette, Dec 2016 - Jan 2017
   Purpose; Apply dynamic range compression to the audio stream.
            Assumes floating-point data.

   This processes a single stream fo audio data (ie, it is mono)

   MIT License.  use at your own risk.
*/

#include <circle/logger.h>
#include <cstdlib>
#include "effect_compressor.h"

LOGMODULE ("compressor");

Compressor::Compressor(const float32_t sample_rate_Hz) {
	  //setDefaultValues(AUDIO_SAMPLE_RATE);   resetStates();
	  setDefaultValues(sample_rate_Hz);
          resetStates();
}
	
void Compressor::setDefaultValues(const float32_t sample_rate_Hz) {
      setThresh_dBFS(-20.0f);     //set the default value for the threshold for compression
      setCompressionRatio(5.0f);  //set the default copression ratio
      setAttack_sec(0.005f, sample_rate_Hz);  //default to this value
      setRelease_sec(0.200f, sample_rate_Hz); //default to this value
      setHPFilterCoeff();  enableHPFilter(true);  //enable the HP filter to remove any DC offset from the audio
}

//Compute the instantaneous desired gain, including the compression ratio and
//threshold for where the comrpession kicks in
void Compressor::calcInstantaneousTargetGain(float32_t *audio_level_dB_block, float32_t *inst_targ_gain_dB_block, uint16_t len)
{
      // how much are we above the compression threshold?
      float32_t above_thresh_dB_block[len];

      //arm_copy_f32(zeroblock_f32,above_thresh_dB_block,len);

      arm_offset_f32(audio_level_dB_block,  //CMSIS DSP for "add a constant value to all elements"
        -thresh_dBFS,                         //this is the value to be added
        above_thresh_dB_block,          //this is the output
        len);  

      // scale by the compression ratio...this is what the output level should be (this is our target level)
      arm_scale_f32(above_thresh_dB_block,    //CMSIS DSP for "multiply all elements by a constant value"
           1.0f / comp_ratio,                       //this is the value to be multiplied 
           inst_targ_gain_dB_block,           //this is the output
           len); 

      // compute the instantaneous gain...which is the difference between the target level and the original level
      arm_sub_f32(inst_targ_gain_dB_block,  //CMSIS DSP for "subtract two vectors element-by-element"
           above_thresh_dB_block,           //this is the vector to be subtracted
           inst_targ_gain_dB_block,         //this is the output
           len);

      // limit the target gain to attenuation only (this part of the compressor should not make things louder!)
      for (uint16_t i=0; i < len; i++) {
        if (inst_targ_gain_dB_block[i] > 0.0f) inst_targ_gain_dB_block[i] = 0.0f;
      }

      return;  //output is passed through inst_targ_gain_dB_block
}

//this method applies the "attack" and "release" constants to smooth the
//target gain level through time.
void Compressor::calcSmoothedGain_dB(float32_t *inst_targ_gain_dB_block, float32_t *gain_dB_block, uint16_t len)
{
      float32_t gain_dB;
      float32_t one_minus_attack_const = 1.0f - attack_const;
      float32_t one_minus_release_const = 1.0f - release_const;
      for (uint16_t i = 0; i < len; i++) {
        gain_dB = inst_targ_gain_dB_block[i];

        //smooth the gain using the attack or release constants
        if (gain_dB < prev_gain_dB) {  //are we in the attack phase?
          gain_dB_block[i] = attack_const*prev_gain_dB + one_minus_attack_const*gain_dB;
        } else {   //or, we're in the release phase
          gain_dB_block[i] = release_const*prev_gain_dB + one_minus_release_const*gain_dB;
        }

        //save value for the next time through this loop
        prev_gain_dB = gain_dB_block[i];
      }

      return;  //the output here is gain_block
}

// Here's the method that estimates the level of the audio (in dB)
// It squares the signal and low-pass filters to get a time-averaged
// signal power.  It then 
void Compressor::calcAudioLevel_dB(float32_t *wav_block, float32_t *level_dB_block, uint16_t len) { 
    	
      // calculate the instantaneous signal power (square the signal)
      float32_t wav_pow_block[len];

      //arm_copy_f32(zeroblock_f32,wav_pow_block,len);

      arm_mult_f32(wav_block, wav_block, wav_pow_block, len);

      // low-pass filter and convert to dB
      float32_t c1 = level_lp_const, c2 = 1.0f - c1; //prepare constants
      for (uint16_t i = 0; i < len; i++) {
        // first-order low-pass filter to get a running estimate of the average power
        wav_pow_block[i] = c1*prev_level_lp_pow + c2*wav_pow_block[i];
        
        // save the state of the first-order low-pass filter
        prev_level_lp_pow = wav_pow_block[i]; 

        //now convert the signal power to dB (but not yet multiplied by 10.0)
        level_dB_block[i] = log10f_approx(wav_pow_block[i]);
      }

      //limit the amount that the state of the smoothing filter can go toward negative infinity
      if (prev_level_lp_pow < (1.0E-13)) prev_level_lp_pow = 1.0E-13;  //never go less than -130 dBFS 

      //scale the wav_pow_block by 10.0 to complete the conversion to dB
      arm_scale_f32(level_dB_block, 10.0f, level_dB_block, len); //use ARM DSP for speed!

      return; //output is passed through level_dB_block
    }

    //This method computes the desired gain from the compressor, given an estimate
    //of the signal level (in dB)
void Compressor::calcGain(float32_t *audio_level_dB_block, float32_t *gain_block,uint16_t len)
{ 
      //first, calculate the instantaneous target gain based on the compression ratio
      float32_t inst_targ_gain_dB_block[len];
      //arm_copy_f32(zeroblock_f32,inst_targ_gain_dB_block,len);

      calcInstantaneousTargetGain(audio_level_dB_block, inst_targ_gain_dB_block,len);
    
      //second, smooth in time (attack and release) by stepping through each sample
      float32_t gain_dB_block[len];
      //arm_copy_f32(zeroblock_f32,gain_dB_block,len);

      calcSmoothedGain_dB(inst_targ_gain_dB_block,gain_dB_block, len);

      //finally, convert from dB to linear gain: gain = 10^(gain_dB/20);  (ie this takes care of the sqrt, too!)
      arm_scale_f32(gain_dB_block, 1.0f/20.0f, gain_dB_block, len);  //divide by 20 
      for (uint16_t i = 0; i < len; i++) gain_block[i] = pow10f(gain_dB_block[i]); //do the 10^(x)
      
      return;  //output is passed through gain_block
}
      
//here's the method that does all the work
void Compressor::doCompression(float32_t *audio_block, uint16_t len) {
      //Serial.println("AudioEffectGain_F32: updating.");  //for debugging.
      if (!audio_block) {
        LOGERR("No audio_block available for Compressor!");
        return;
      }

      //apply a high-pass filter to get rid of the DC offset
      if (use_HP_prefilter)
        arm_biquad_cascade_df1_f32(&hp_filt_struct, audio_block, audio_block, len);
      
      //apply the pre-gain...a negative gain value will disable
      if (pre_gain > 0.0f)
        arm_scale_f32(audio_block, pre_gain, audio_block, len); //use ARM DSP for speed!

      //calculate the level of the audio (ie, calculate a smoothed version of the signal power)
      float32_t audio_level_dB_block[len];

      //arm_copy_f32(zeroblock_f32,audio_level_dB_block,len);

      calcAudioLevel_dB(audio_block, audio_level_dB_block, len); //returns through audio_level_dB_block

      //compute the desired gain based on the observed audio level
      float32_t gain_block[len];

      //arm_copy_f32(zeroblock_f32,gain_block,len);

      calcGain(audio_level_dB_block, gain_block, len);  //returns through gain_block

      //apply the desired gain...store the processed audio back into audio_block
      arm_mult_f32(audio_block, gain_block, audio_block, len);
}

//methods to set parameters of this module
void Compressor::resetStates(void)
{
      prev_level_lp_pow = 1.0f;
      prev_gain_dB = 0.0f;
      
      //initialize the HP filter.  (This also resets the filter states,)
      arm_biquad_cascade_df1_init_f32(&hp_filt_struct, hp_nstages, hp_coeff, hp_state);
}

void Compressor::setPreGain(float32_t g)
{
    pre_gain = g;
}

void Compressor::setPreGain_dB(float32_t gain_dB)
{
    setPreGain(pow(10.0, gain_dB / 20.0));
}

void Compressor::setCompressionRatio(float32_t cr)
{
      comp_ratio = max(0.001f, cr); //limit to positive values
      updateThresholdAndCompRatioConstants();
}

void Compressor::setAttack_sec(float32_t a, float32_t fs_Hz)
{
      attack_sec = a;
      attack_const = expf(-1.0f / (attack_sec * fs_Hz)); //expf() is much faster than exp()

      //also update the time constant for the envelope extraction
      setLevelTimeConst_sec(min(attack_sec,release_sec) / 5.0, fs_Hz);  //make the level time-constant one-fifth the gain time constants
} 

void Compressor::setRelease_sec(float32_t r, float32_t fs_Hz)
{
      release_sec = r;
      release_const = expf(-1.0f / (release_sec * fs_Hz)); //expf() is much faster than exp()

      //also update the time constant for the envelope extraction
      setLevelTimeConst_sec(min(attack_sec,release_sec) / 5.0, fs_Hz);  //make the level time-constant one-fifth the gain time constants
}

void Compressor::setLevelTimeConst_sec(float32_t t_sec, float32_t fs_Hz)
{
      const float32_t min_t_sec = 0.002f;  //this is the minimum allowed value
      level_lp_sec = max(min_t_sec,t_sec);
      level_lp_const = expf(-1.0f / (level_lp_sec * fs_Hz)); //expf() is much faster than exp()
}

void Compressor::setThresh_dBFS(float32_t val)
{ 
      thresh_dBFS = val;
      setThreshPow(pow(10.0, thresh_dBFS / 10.0));
}

void Compressor::enableHPFilter(boolean flag)
{
      use_HP_prefilter = flag;
}

void Compressor::setHPFilterCoeff_N2IIR_Matlab(float32_t b[], float32_t a[])
{
      //https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html#ga8e73b69a788e681a61bccc8959d823c5
      //Use matlab to compute the coeff for HP at 20Hz: [b,a]=butter(2,20/(44100/2),'high'); %assumes fs_Hz = 44100
      hp_coeff[0] = b[0];   hp_coeff[1] = b[1];  hp_coeff[2] = b[2]; //here are the matlab "b" coefficients
      hp_coeff[3] = -a[1];  hp_coeff[4] = -a[2];  //the DSP needs the "a" terms to have opposite sign vs Matlab    	
}
    
void Compressor::setHPFilterCoeff(void)
{
      //https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html#ga8e73b69a788e681a61bccc8959d823c5
      //Use matlab to compute the coeff for HP at 20Hz: [b,a]=butter(2,20/(44100/2),'high'); %assumes fs_Hz = 44100
      float32_t b[] = {9.979871156751189e-01,    -1.995974231350238e+00, 9.979871156751189e-01};  //from Matlab
      float32_t a[] = { 1.000000000000000e+00,    -1.995970179642828e+00,    9.959782830576472e-01};  //from Matlab
      setHPFilterCoeff_N2IIR_Matlab(b, a);
      //hp_coeff[0] = b[0];   hp_coeff[1] = b[1];  hp_coeff[2] = b[2]; //here are the matlab "b" coefficients
      //hp_coeff[3] = -a[1];  hp_coeff[4] = -a[2];  //the DSP needs the "a" terms to have opposite sign vs Matlab
}

void Compressor::updateThresholdAndCompRatioConstants(void)
{
      comp_ratio_const = 1.0f-(1.0f / comp_ratio);
      thresh_pow_FS_wCR = powf(thresh_pow_FS, comp_ratio_const);    
}

void Compressor::setThreshPow(float32_t t_pow)
{ 
      thresh_pow_FS = t_pow;
      updateThresholdAndCompRatioConstants();
}
    
// Accelerate the powf(10.0,x) function
static float32_t pow10f(float32_t x)
{
      //return powf(10.0f,x)   //standard, but slower
      return expf(2.302585092994f*x);  //faster:  exp(log(10.0f)*x)
}

// Accelerate the log10f(x)  function?
static float32_t log10f_approx(float32_t x)
{
      //return log10f(x);   //standard, but slower
      return log2f_approx(x)*0.3010299956639812f; //faster:  log2(x)/log2(10)
}
    
/* ----------------------------------------------------------------------
** Fast approximation to the log2() function.  It uses a two step
** process.  First, it decomposes the floating-point number into
** a fractional component F and an exponent E.  The fraction component
** is used in a polynomial approximation and then the exponent added
** to the result.  A 3rd order polynomial is used and the result
** when computing db20() is accurate to 7.984884e-003 dB.
** ------------------------------------------------------------------- */
//https://community.arm.com/tools/f/discussions/4292/cmsis-dsp-new-functionality-proposal/22621#22621
//float32_t log2f_approx_coeff[4] = {1.23149591368684f, -4.11852516267426f, 6.02197014179219f, -3.13396450166353f};
static float32_t log2f_approx(float32_t X)
{
      //float32_t *C = &log2f_approx_coeff[0];
      float32_t Y;
      float32_t F;
      int E;
    
      // This is the approximation to log2()
      F = frexpf(fabsf(X), &E);
      //  Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
      //Y = *C++;
      Y = 1.23149591368684f;
      Y *= F;
      //Y += (*C++);
      Y += -4.11852516267426f;
      Y *= F;
      //Y += (*C++);
      Y += 6.02197014179219f;
      Y *= F;
      //Y += (*C++);
      Y += -3.13396450166353f;
      Y += E;
    
      return(Y);
}
