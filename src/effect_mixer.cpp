// Taken from https://github.com/manicken/Audio/tree/templateMixer
// Adapted for MiniDexed by Holger Wirtz <dcoredump@googlemail.com>

/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
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

#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include "arm_math.h"
#include "effect_mixer.h"

template <int NN> void AudioMixer<NN>::gain(uint8_t channel, float32_t gain)
{
    if (channel >= NN) return;

    if (gain > MAX_GAIN)
         gain = MAX_GAIN;
    else if (gain < MIN_GAIN)
         gain = MIN_GAIN;
    multiplier[channel] = gain;
}

template <int NN> void AudioMixer<NN>::gain(float32_t gain)
{
    for (uint8_t i = 0; i < NN; i++)
    {
        if (gain > MAX_GAIN)
            gain = MAX_GAIN;
        else if (gain < MIN_GAIN)
            gain = MIN_GAIN;
        multiplier[i] = gain;
    } 
}

template <int NN> void AudioMixer<NN>::doAddMix(uint8_t channel, float32_t* in, float32_t* out, uint16_t len)
{
    float32_t* tmp=malloc(sizeof(float32_t)*len);

    assert(tmp!=NULL);

    arm_scale_f32(in,multiplier[channel],tmp,len);
    arm_add_f32(out, tmp, out, len);

    free(tmp);
}

template <int NN> void AudioMixer<NN>::get_mix(float32_t* buffer, uint16_t len)
{
    assert(buffer);
    assert(sumbuf);
    arm_copy_f32 (sumbuf, buffer, len);
}

template <int NN> void AudioStereoMixer<NN>::doAddMix(uint8_t channel, float32_t* inL, float32_t* inR, float32_t* outL, float32_t* outR, uint16_t len)
{
    float32_t* tmp=malloc(sizeof(float32_t)*len);

    assert(tmp!=NULL);

    // panorama
    for(uint16_t i=0;i<len;i++)
    {
	// left
    	arm_scale_f32(inL,AudioMixer<NN>::multiplier[channel],tmp,len);
    	arm_add_f32(outL, tmp, outL, len);
	// right
    	arm_scale_f32(inR,AudioMixer<NN>::multiplier[channel],tmp,len);
    	arm_add_f32(outR, tmp, outR, len);
    }

    free(tmp);
}

template <int NN> void AudioMixer<NN>::get_mix(float32_t* bufferL, float32_t bufferL, uint16_t len)
{
    assert(bufferR);
    assert(bufferL);
    assert(sumbuf);
    arm_copy_f32 (sumbuf[0], bufferL, len);
    arm_copy_f32 (sumbuf[1], bufferR, len);
}
