/*  Stereo plate reverb for Teensy 4
 *
 *  Adapted for MiniDexed (Holger Wirtz <dcoredump@googlemail.com>)
 *
 *  Author: Piotr Zapart
 *          www.hexefx.com
 *
 * Copyright (c) 2020 by Piotr Zapart
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include "effect_platervbstereo.h"

#define INP_ALLP_COEFF      (0.65f)                         // default input allpass coeff
#define LOOP_ALLOP_COEFF    (0.65f)                         // default loop allpass coeff

#define HI_LOSS_FREQ        (0.3f)                          // scaled center freq for the treble loss filter 
// #define HI_LOSS_FREQ_MAX    (0.08f)
#define LO_LOSS_FREQ        (0.06f)                         // scaled center freq for the bass loss filter 

#define LFO_AMPL_BITS       (5)                             // 2^LFO_AMPL_BITS will be the LFO amplitude 
#define LFO_AMPL            ((1<<LFO_AMPL_BITS) + 1)        // lfo amplitude
#define LFO_READ_OFFSET     (LFO_AMPL>>1)                   // read offset = half the amplitude
#define LFO_FRAC_BITS       (16 - LFO_AMPL_BITS)            // fractional part used for linear interpolation
#define LFO_FRAC_MASK       ((1<<LFO_FRAC_BITS)-1)          // mask for the above

#define LFO1_FREQ_HZ        (1.37f)                          // LFO1 frequency in Hz
#define LFO2_FREQ_HZ        (1.52f)                          // LFO2 frequency in Hz

#define RV_MASTER_LOWPASS_F (0.6f)                           // master lowpass scaled frequency coeff. 

const int16_t AudioWaveformSine[257] = {
     0,   804,  1608,  2410,  3212,  4011,  4808,  5602,  6393,  7179,
  7962,  8739,  9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732,
 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403,
 22005, 22594, 23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956, 30273, 30571,
 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521,
 32609, 32678, 32728, 32757, 32767, 32757, 32728, 32678, 32609, 32521,
 32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
 30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790,
 26319, 25832, 25329, 24811, 24279, 23731, 23170, 22594, 22005, 21403,
 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732,
 14010, 13279, 12539, 11793, 11039, 10278,  9512,  8739,  7962,  7179,
  6393,  5602,  4808,  4011,  3212,  2410,  1608,   804,     0,  -804,
 -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739,
 -9512,-10278,-11039,-11793,-12539,-13279,-14010,-14732,-15446,-16151,
-16846,-17530,-18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,
-23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,-27245,-27683,
-28105,-28510,-28898,-29268,-29621,-29956,-30273,-30571,-30852,-31113,
-31356,-31580,-31785,-31971,-32137,-32285,-32412,-32521,-32609,-32678,
-32728,-32757,-32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,
-32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,-30273,-29956,
-29621,-29268,-28898,-28510,-28105,-27683,-27245,-26790,-26319,-25832,
-25329,-24811,-24279,-23731,-23170,-22594,-22005,-21403,-20787,-20159,
-19519,-18868,-18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,
-12539,-11793,-11039,-10278, -9512, -8739, -7962, -7179, -6393, -5602,
 -4808, -4011, -3212, -2410, -1608,  -804,     0
};

AudioEffectPlateReverb::AudioEffectPlateReverb(float32_t samplerate)
{
    input_attn = 0.5f;
    in_allp_k = INP_ALLP_COEFF;

    memset(in_allp1_bufL, 0, sizeof(in_allp1_bufL));
    memset(in_allp2_bufL, 0, sizeof(in_allp2_bufL));
    memset(in_allp3_bufL, 0, sizeof(in_allp3_bufL));
    memset(in_allp4_bufL, 0, sizeof(in_allp4_bufL));
    in_allp1_idxL = 0;
    in_allp2_idxL = 0;
    in_allp3_idxL = 0;
    in_allp4_idxL = 0;

    memset(in_allp1_bufR, 0, sizeof(in_allp1_bufR));
    memset(in_allp2_bufR, 0, sizeof(in_allp2_bufR));
    memset(in_allp3_bufR, 0, sizeof(in_allp3_bufR));
    memset(in_allp4_bufR, 0, sizeof(in_allp4_bufR));
    in_allp1_idxR = 0;
    in_allp2_idxR = 0;
    in_allp3_idxR = 0;
    in_allp4_idxR = 0;

    in_allp_out_R = 0.0f;

    memset(lp_allp1_buf, 0, sizeof(lp_allp1_buf));
    memset(lp_allp2_buf, 0, sizeof(lp_allp2_buf));
    memset(lp_allp3_buf, 0, sizeof(lp_allp3_buf));
    memset(lp_allp4_buf, 0, sizeof(lp_allp4_buf));
    lp_allp1_idx = 0;
    lp_allp2_idx = 0;
    lp_allp3_idx = 0;
    lp_allp4_idx = 0;
    loop_allp_k = LOOP_ALLOP_COEFF;
    lp_allp_out = 0.0f;

    memset(lp_dly1_buf, 0, sizeof(lp_dly1_buf));
    memset(lp_dly2_buf, 0, sizeof(lp_dly2_buf));
    memset(lp_dly3_buf, 0, sizeof(lp_dly3_buf));
    memset(lp_dly4_buf, 0, sizeof(lp_dly4_buf));
    lp_dly1_idx = 0;
    lp_dly2_idx = 0;
    lp_dly3_idx = 0;
    lp_dly4_idx = 0;

    lp_hidamp_k = 1.0f;
    lp_lodamp_k = 0.0f;

    lp_lowpass_f = HI_LOSS_FREQ;
    lp_hipass_f = LO_LOSS_FREQ;

    lpf1 = 0.0f;
    lpf2 = 0.0f;
    lpf3 = 0.0f;
    lpf4 = 0.0f;

    hpf1 = 0.0f;
    hpf2 = 0.0f;
    hpf3 = 0.0f;
    hpf4 = 0.0f;

    master_lowpass_f = RV_MASTER_LOWPASS_F;
    master_lowpass_l = 0.0f;
    master_lowpass_r = 0.0f;

    lfo1_phase_acc = 0;
    lfo1_adder = (UINT32_MAX + 1)/(samplerate * LFO1_FREQ_HZ);
    lfo2_phase_acc = 0;
    lfo2_adder = (UINT32_MAX + 1)/(samplerate * LFO2_FREQ_HZ);  

    reverb_level = 0.0f;
}

// #define sat16(n, rshift) signed_saturate_rshift((n), 16, (rshift))

void AudioEffectPlateReverb::doReverb(const float32_t* inblockL, const float32_t* inblockR, float32_t* rvbblockL, float32_t* rvbblockR, uint16_t len)
{
    float32_t input, acc, temp1, temp2;
    uint16_t temp16;
    float32_t rv_time;

    // for LFOs:
    int16_t lfo1_out_sin, lfo1_out_cos, lfo2_out_sin, lfo2_out_cos;
    int32_t y0, y1;
    int64_t y;
    uint32_t idx;
    static bool cleanup_done = false;

    // handle bypass, 1st call will clean the buffers to avoid continuing the previous reverb tail
    if (bypass)
    {
        if (!cleanup_done)
        {
            memset(in_allp1_bufL, 0, sizeof(in_allp1_bufL));
            memset(in_allp2_bufL, 0, sizeof(in_allp2_bufL));
            memset(in_allp3_bufL, 0, sizeof(in_allp3_bufL));
            memset(in_allp4_bufL, 0, sizeof(in_allp4_bufL));
            memset(in_allp1_bufR, 0, sizeof(in_allp1_bufR));
            memset(in_allp2_bufR, 0, sizeof(in_allp2_bufR));
            memset(in_allp3_bufR, 0, sizeof(in_allp3_bufR));
            memset(in_allp4_bufR, 0, sizeof(in_allp4_bufR));
            memset(lp_allp1_buf, 0, sizeof(lp_allp1_buf));
            memset(lp_allp2_buf, 0, sizeof(lp_allp2_buf));
            memset(lp_allp3_buf, 0, sizeof(lp_allp3_buf));
            memset(lp_allp4_buf, 0, sizeof(lp_allp4_buf));
            memset(lp_dly1_buf, 0, sizeof(lp_dly1_buf));
            memset(lp_dly2_buf, 0, sizeof(lp_dly2_buf));
            memset(lp_dly3_buf, 0, sizeof(lp_dly3_buf));
            memset(lp_dly4_buf, 0, sizeof(lp_dly4_buf));

            cleanup_done = true;
        }

        return;
    }
    cleanup_done = false;

    rv_time = rv_time_k;

    for (uint16_t i=0; i < len; i++) 
    {
        // do the LFOs
        lfo1_phase_acc += lfo1_adder;
        idx = lfo1_phase_acc >> 24;     // 8bit lookup table address
        y0 =  AudioWaveformSine[idx];
        y1 = AudioWaveformSine[idx+1];
        idx = lfo1_phase_acc & 0x00FFFFFF;   // lower 24 bit = fractional part
        y = (int64_t)y0 * (0x00FFFFFF - idx);
        y += (int64_t)y1 * idx;
        lfo1_out_sin = (int32_t) (y >> (32-8)); // 16bit output
        idx = ((lfo1_phase_acc >> 24)+64) & 0xFF;
        y0 = AudioWaveformSine[idx];
        y1 = AudioWaveformSine[idx + 1];
        y = (int64_t)y0 * (0x00FFFFFF - idx);
        y += (int64_t)y1 * idx;
        lfo1_out_cos = (int32_t) (y >> (32-8)); // 16bit output        

        lfo2_phase_acc += lfo2_adder;
        idx = lfo2_phase_acc >> 24;     // 8bit lookup table address
        y0 =  AudioWaveformSine[idx];
        y1 = AudioWaveformSine[idx+1];
        idx = lfo2_phase_acc & 0x00FFFFFF;   // lower 24 bit = fractional part
        y = (int64_t)y0 * (0x00FFFFFF - idx);
        y += (int64_t)y1 * idx;
        lfo2_out_sin = (int32_t) (y >> (32-8)); //32-8->output 16bit,
        idx = ((lfo2_phase_acc >> 24)+64) & 0xFF;
        y0 = AudioWaveformSine[idx];
        y1 = AudioWaveformSine[idx + 1];
        y = (int64_t)y0 * (0x00FFFFFF - idx);
        y += (int64_t)y1 * idx;
        lfo2_out_cos = (int32_t) (y >> (32-8)); // 16bit output   

	input = inblockL[i] * input_attn;

        // chained input allpasses, channel L
        acc = in_allp1_bufL[in_allp1_idxL]  + input * in_allp_k;  
        in_allp1_bufL[in_allp1_idxL] = input - in_allp_k * acc;
        input = acc;
        if (++in_allp1_idxL >= sizeof(in_allp1_bufL)/sizeof(float32_t)) in_allp1_idxL = 0;

        acc = in_allp2_bufL[in_allp2_idxL]  + input * in_allp_k;  
        in_allp2_bufL[in_allp2_idxL] = input - in_allp_k * acc;
        input = acc;
        if (++in_allp2_idxL >= sizeof(in_allp2_bufL)/sizeof(float32_t)) in_allp2_idxL = 0;

        acc = in_allp3_bufL[in_allp3_idxL]  + input * in_allp_k;  
        in_allp3_bufL[in_allp3_idxL] = input - in_allp_k * acc;
        input = acc;
        if (++in_allp3_idxL >= sizeof(in_allp3_bufL)/sizeof(float32_t)) in_allp3_idxL = 0;

        acc = in_allp4_bufL[in_allp4_idxL]  + input * in_allp_k;  
        in_allp4_bufL[in_allp4_idxL] = input - in_allp_k * acc;
        in_allp_out_L = acc;
        if (++in_allp4_idxL >= sizeof(in_allp4_bufL)/sizeof(float32_t)) in_allp4_idxL = 0;

        input = inblockR[i] * input_attn;

        // chained input allpasses, channel R
        acc = in_allp1_bufR[in_allp1_idxR]  + input * in_allp_k;  
        in_allp1_bufR[in_allp1_idxR] = input - in_allp_k * acc;
        input = acc;
        if (++in_allp1_idxR >= sizeof(in_allp1_bufR)/sizeof(float32_t)) in_allp1_idxR = 0;

        acc = in_allp2_bufR[in_allp2_idxR]  + input * in_allp_k;  
        in_allp2_bufR[in_allp2_idxR] = input - in_allp_k * acc;
        input = acc;
        if (++in_allp2_idxR >= sizeof(in_allp2_bufR)/sizeof(float32_t)) in_allp2_idxR = 0;

        acc = in_allp3_bufR[in_allp3_idxR]  + input * in_allp_k;  
        in_allp3_bufR[in_allp3_idxR] = input - in_allp_k * acc;
        input = acc;
        if (++in_allp3_idxR >= sizeof(in_allp3_bufR)/sizeof(float32_t)) in_allp3_idxR = 0;

        acc = in_allp4_bufR[in_allp4_idxR]  + input * in_allp_k;  
        in_allp4_bufR[in_allp4_idxR] = input - in_allp_k * acc;
        in_allp_out_R = acc;
        if (++in_allp4_idxR >= sizeof(in_allp4_bufR)/sizeof(float32_t)) in_allp4_idxR = 0;

        // input allpases done, start loop allpases
        input = lp_allp_out + in_allp_out_R; 
        acc = lp_allp1_buf[lp_allp1_idx] + input * loop_allp_k;                  // input is the lp allpass chain output
        lp_allp1_buf[lp_allp1_idx] = input - loop_allp_k * acc;
        input = acc;
        if (++lp_allp1_idx >= sizeof(lp_allp1_buf)/sizeof(float32_t)) lp_allp1_idx = 0;
        
        acc = lp_dly1_buf[lp_dly1_idx];                                                   // read the end of the delay
        lp_dly1_buf[lp_dly1_idx] = input;                                                 // write new sample
        input = acc;
        if (++lp_dly1_idx >= sizeof(lp_dly1_buf)/sizeof(float32_t)) lp_dly1_idx = 0;     // update index

        // hi/lo shelving filter
        temp1 = input - lpf1;
        lpf1 += temp1 * lp_lowpass_f;
        temp2 = input - lpf1;
        temp1 = lpf1 - hpf1;
        hpf1 += temp1 * lp_hipass_f;
        acc = lpf1 + temp2*lp_hidamp_k + hpf1*lp_lodamp_k;
        acc = acc * rv_time * rv_time_scaler;                                                                // scale by the reveb time
        
        input = acc + in_allp_out_L;

        acc = lp_allp2_buf[lp_allp2_idx] + input * loop_allp_k;                  
        lp_allp2_buf[lp_allp2_idx] = input - loop_allp_k * acc;
        input = acc;
        if (++lp_allp2_idx >= sizeof(lp_allp2_buf)/sizeof(float32_t)) lp_allp2_idx = 0;
        acc = lp_dly2_buf[lp_dly2_idx];                                                   // read the end of the delay
        lp_dly2_buf[lp_dly2_idx] = input;                                                 // write new sample
        input = acc;
        if (++lp_dly2_idx >= sizeof(lp_dly2_buf)/sizeof(float32_t)) lp_dly2_idx = 0;     // update index
        // hi/lo shelving filter
        temp1 = input - lpf2;
        lpf2 += temp1 * lp_lowpass_f;
        temp2 = input - lpf2;
        temp1 = lpf2 - hpf2;
        hpf2 += temp1 * lp_hipass_f;
        acc = lpf2 + temp2*lp_hidamp_k + hpf2*lp_lodamp_k;
        acc = acc * rv_time * rv_time_scaler;             

        input = acc + in_allp_out_R;

        acc = lp_allp3_buf[lp_allp3_idx] + input * loop_allp_k;                  
        lp_allp3_buf[lp_allp3_idx] = input - loop_allp_k * acc;
        input = acc;
        if (++lp_allp3_idx >= sizeof(lp_allp3_buf)/sizeof(float32_t)) lp_allp3_idx = 0;
        acc = lp_dly3_buf[lp_dly3_idx];                                                   // read the end of the delay
        lp_dly3_buf[lp_dly3_idx] = input;                                                 // write new sample
        input = acc;
        if (++lp_dly3_idx >= sizeof(lp_dly3_buf)/sizeof(float32_t)) lp_dly3_idx = 0;     // update index
        // hi/lo shelving filter
        temp1 = input - lpf3;
        lpf3 += temp1 * lp_lowpass_f;
        temp2 = input - lpf3;
        temp1 = lpf3 - hpf3;
        hpf3 += temp1 * lp_hipass_f;
        acc = lpf3 + temp2*lp_hidamp_k + hpf3*lp_lodamp_k;
        acc = acc * rv_time * rv_time_scaler;              

        input = acc + in_allp_out_L;       

        acc = lp_allp4_buf[lp_allp4_idx] + input * loop_allp_k;                  
        lp_allp4_buf[lp_allp4_idx] = input - loop_allp_k * acc;
        input = acc;
        if (++lp_allp4_idx >= sizeof(lp_allp4_buf)/sizeof(float32_t)) lp_allp4_idx = 0;
        acc = lp_dly4_buf[lp_dly4_idx];                                                   // read the end of the delay
        lp_dly4_buf[lp_dly4_idx] = input;                                                 // write new sample
        input = acc;
        if (++lp_dly4_idx >= sizeof(lp_dly4_buf)/sizeof(float32_t)) lp_dly4_idx= 0;     // update index
        // hi/lo shelving filter
        temp1 = input - lpf4;
        lpf4 += temp1 * lp_lowpass_f;
        temp2 = input - lpf4;
        temp1 = lpf4 - hpf4;
        hpf4 += temp1 * lp_hipass_f;
        acc = lpf4 + temp2*lp_hidamp_k + hpf4*lp_lodamp_k;
        acc = acc * rv_time * rv_time_scaler;              

        lp_allp_out = acc;

        // channel L:
#ifdef TAP1_MODULATED
        temp16 = (lp_dly1_idx + lp_dly1_offset_L + (lfo1_out_cos>>LFO_FRAC_BITS)) %  (sizeof(lp_dly1_buf)/sizeof(float32_t));
        temp1 = lp_dly1_buf[temp16++];    // sample now
        if (temp16  >= sizeof(lp_dly1_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly1_buf[temp16];    // sample next
        input = (float32_t)(lfo1_out_cos & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k
        acc = (temp1*(1.0f-input) + temp2*input)* 0.8f;
#else
        temp16 = (lp_dly1_idx + lp_dly1_offset_L) %  (sizeof(lp_dly1_buf)/sizeof(float32_t));
        acc = lp_dly1_buf[temp16]* 0.8f;
#endif


#ifdef TAP2_MODULATED
        temp16 = (lp_dly2_idx + lp_dly2_offset_L + (lfo1_out_sin>>LFO_FRAC_BITS)) % (sizeof(lp_dly2_buf)/sizeof(float32_t));
        temp1 = lp_dly2_buf[temp16++];
        if (temp16  >= sizeof(lp_dly2_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly2_buf[temp16]; 
        input = (float32_t)(lfo1_out_sin & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k
        acc += (temp1*(1.0f-input) + temp2*input)* 0.7f;
#else
        temp16 = (lp_dly2_idx + lp_dly2_offset_L) % (sizeof(lp_dly2_buf)/sizeof(float32_t));
        acc += (temp1*(1.0f-input) + temp2*input)* 0.6f;
#endif

        temp16 = (lp_dly3_idx + lp_dly3_offset_L + (lfo2_out_cos>>LFO_FRAC_BITS)) % (sizeof(lp_dly3_buf)/sizeof(float32_t));
        temp1 = lp_dly3_buf[temp16++];
        if (temp16  >= sizeof(lp_dly3_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly3_buf[temp16]; 
        input = (float32_t)(lfo2_out_cos & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k
        acc += (temp1*(1.0f-input) + temp2*input)* 0.6f;

        temp16 = (lp_dly4_idx + lp_dly4_offset_L + (lfo2_out_sin>>LFO_FRAC_BITS)) % (sizeof(lp_dly4_buf)/sizeof(float32_t));
        temp1 = lp_dly4_buf[temp16++];
        if (temp16  >= sizeof(lp_dly4_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly4_buf[temp16]; 
        input = (float32_t)(lfo2_out_sin & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k
        acc += (temp1*(1.0f-input) + temp2*input)* 0.5f;

        // Master lowpass filter
        temp1 = acc - master_lowpass_l;
        master_lowpass_l += temp1 * master_lowpass_f;

	rvbblockL[i] = master_lowpass_l;

        // Channel R
        #ifdef TAP1_MODULATED
        temp16 = (lp_dly1_idx + lp_dly1_offset_R + (lfo2_out_cos>>LFO_FRAC_BITS)) %  (sizeof(lp_dly1_buf)/sizeof(float32_t));
        temp1 = lp_dly1_buf[temp16++];    // sample now
        if (temp16  >= sizeof(lp_dly1_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly1_buf[temp16];    // sample next
        input = (float32_t)(lfo2_out_cos & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k

        acc = (temp1*(1.0f-input) + temp2*input)* 0.8f;
        #else
        temp16 = (lp_dly1_idx + lp_dly1_offset_R) %  (sizeof(lp_dly1_buf)/sizeof(float32_t));
        acc = lp_dly1_buf[temp16] * 0.8f;
        #endif
#ifdef TAP2_MODULATED
        temp16 = (lp_dly2_idx + lp_dly2_offset_R + (lfo1_out_cos>>LFO_FRAC_BITS)) % (sizeof(lp_dly2_buf)/sizeof(float32_t));
        temp1 = lp_dly2_buf[temp16++];
        if (temp16  >= sizeof(lp_dly2_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly2_buf[temp16]; 
        input = (float32_t)(lfo1_out_cos & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k
        acc += (temp1*(1.0f-input) + temp2*input)* 0.7f;
#else
        temp16 = (lp_dly2_idx + lp_dly2_offset_R) % (sizeof(lp_dly2_buf)/sizeof(float32_t));
        acc += (temp1*(1.0f-input) + temp2*input)* 0.7f;
#endif
        temp16 = (lp_dly3_idx + lp_dly3_offset_R + (lfo2_out_sin>>LFO_FRAC_BITS)) % (sizeof(lp_dly3_buf)/sizeof(float32_t));
        temp1 = lp_dly3_buf[temp16++];
        if (temp16  >= sizeof(lp_dly3_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly3_buf[temp16]; 
        input = (float32_t)(lfo2_out_sin & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k
        acc += (temp1*(1.0f-input) + temp2*input)* 0.6f;

        temp16 = (lp_dly4_idx + lp_dly4_offset_R + (lfo1_out_sin>>LFO_FRAC_BITS)) % (sizeof(lp_dly4_buf)/sizeof(float32_t));
        temp1 = lp_dly4_buf[temp16++];
        if (temp16  >= sizeof(lp_dly4_buf)/sizeof(float32_t)) temp16 = 0;
        temp2 = lp_dly4_buf[temp16]; 
        input = (float32_t)(lfo2_out_cos & LFO_FRAC_MASK) / ((float32_t)LFO_FRAC_MASK); // interp. k
        acc += (temp1*(1.0f-input) + temp2*input)* 0.5f;

        // Master lowpass filter
        temp1 = acc - master_lowpass_r;
        master_lowpass_r += temp1 * master_lowpass_f;

	rvbblockR[i] = master_lowpass_r;
    }
}
