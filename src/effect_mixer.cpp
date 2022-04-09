// Taken from https://github.com/manicken/Audio/tree/templateMixer
// Adapted for MiniDexed by Holger Wirtz <dcoredump@googlemail.com>

#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include "arm_math.h"
#include "effect_mixer.h"

template <int NN> AudioMixer<NN>::AudioMixer(uint16_t len)
{
    buffer_length=len;
    for (uint8_t i=0; i<NN; i++)
      multiplier[i] = UNITY_GAIN;

    sumbufL=(float32_t*)malloc(sizeof(float32_t) * buffer_length);
    arm_fill_f32(0.0, sumbufL, len);
}

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

template <int NN> void AudioMixer<NN>::doAddMix(uint8_t channel, float32_t* in)
{
    float32_t* tmp=malloc(sizeof(float32_t)*buffer_length);

    assert(tmp!=NULL);
    assert(in);

    if(multiplier[channel]!=UNITY_GAIN)
      arm_scale_f32(in,multiplier[channel],tmp,buffer_length);
    arm_add_f32(sumbufL, tmp, sumbufL, buffer_length);

    if(sumbufL)
      arm_fill_f32(0.0, sumbufL, buffer_length);

    free(tmp);
}

template <int NN> void AudioMixer<NN>::getMix(float32_t* buffer)
{
    assert(buffer);
    assert(sumbufL);
    arm_copy_f32(sumbufL, buffer, buffer_length);
}

template <int NN> AudioStereoMixer<NN>::AudioStereoMixer(uint16_t len) : AudioMixer<NN>(len)
{
    buffer_length=len;
    for (uint8_t i=0; i<NN; i++)
      panorama[i] = UNITY_PANORAMA;

    sumbufR=(float32_t*)malloc(sizeof(float32_t) * buffer_length);
    arm_fill_f32(0.0, sumbufR, buffer_length);
}

template <int NN> void AudioStereoMixer<NN>::pan(uint8_t channel, float32_t pan)
{
    if (channel >= NN) return;

    if (pan > MAX_PANORAMA)
         pan = MAX_PANORAMA;
    else if (pan < MIN_PANORAMA)
         pan = MIN_PANORAMA;
    panorama[channel] = pan;
}

template <int NN> void AudioStereoMixer<NN>::doAddMix(uint8_t channel, float32_t* in)
{
    float32_t* tmp=malloc(sizeof(float32_t)*buffer_length);

    assert(tmp!=NULL);
    assert(in);

    // left
    arm_scale_f32(in, 1.0f-panorama[channel], tmp, buffer_length);
    if(multiplier[channel]!=UNITY_GAIN)
      arm_scale_f32(tmp,AudioMixer<NN>::multiplier[channel],tmp,buffer_length);
    arm_add_f32(sumbufL, tmp, sumbufL, buffer_length);
    // right
    arm_scale_f32(in, panorama[channel], tmp, buffer_length);
    if(multiplier[channel]!=UNITY_GAIN)
       arm_scale_f32(tmp,AudioMixer<NN>::multiplier[channel],tmp,buffer_length);
    arm_add_f32(sumbufR, tmp, sumbufR, buffer_length);

    if(sumbufL)
      arm_fill_f32(0.0, sumbufL, buffer_length);
    if(sumbufR)
      arm_fill_f32(0.0, sumbufR, buffer_length);

    free(tmp);
}

template <int NN> void AudioStereoMixer<NN>::doAddMix(uint8_t channel, float32_t* inL, float32_t* inR)
{
    float32_t* tmp=malloc(sizeof(float32_t)*buffer_length);

    assert(tmp!=NULL);
    assert(inL);
    assert(inR);

    // left
    if(multiplier[channel]!=UNITY_GAIN)
      arm_scale_f32(inL,AudioMixer<NN>::multiplier[channel],tmp,buffer_length);
    arm_add_f32(sumbufL, tmp, sumbufL, buffer_length);
    // right
    if(multiplier[channel]!=UNITY_GAIN)
       arm_scale_f32(inR,AudioMixer<NN>::multiplier[channel],tmp,buffer_length);
    arm_add_f32(sumbufR, tmp, sumbufR, buffer_length);

    free(tmp);
}

template <int NN> void AudioStereoMixer<NN>::getMix(float32_t* bufferL, float32_t* bufferR)
{
    assert(bufferR);
    assert(bufferL);
    assert(sumbufL);
    assert(sumbufR);
    arm_copy_f32 (sumbufL, bufferL, buffer_length);
    arm_copy_f32 (sumbufR, bufferR, buffer_length);

    if(sumbufL)
      arm_fill_f32(0.0, sumbufL, buffer_length);
    if(sumbufR)
      arm_fill_f32(0.0, sumbufR, buffer_length);

}
