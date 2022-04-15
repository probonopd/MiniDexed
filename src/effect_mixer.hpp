// Taken from https://github.com/manicken/Audio/tree/templateMixer
// Adapted for MiniDexed by Holger Wirtz <dcoredump@googlemail.com>

#ifndef effect_mixer_h_
#define effect_mixer_h_

#include <cstdint>
#include <assert.h>
#include "arm_math.h"

#define UNITY_GAIN 1.0f
#define MAX_GAIN 1.0f
#define MIN_GAIN 0.0f
#define UNITY_PANORAMA 1.0f
#define MAX_PANORAMA 1.0f
#define MIN_PANORAMA 0.0f

template <int NN> class AudioMixer
{
public:
	AudioMixer(uint16_t len)
	{
		buffer_length=len;
		for (uint8_t i=0; i<NN; i++)
			multiplier[i] = UNITY_GAIN;

		sumbufL=new float32_t[buffer_length];
		arm_fill_f32(0.0f, sumbufL, len);
	}

	~AudioMixer()
	{
		delete [] sumbufL;
	}

        void doAddMix(uint8_t channel, float32_t* in)
	{
		float32_t tmp[buffer_length];

		assert(in);

		if(multiplier[channel]!=UNITY_GAIN)
			arm_scale_f32(in,multiplier[channel],tmp,buffer_length);
		arm_add_f32(sumbufL, tmp, sumbufL, buffer_length);
	}

	void gain(uint8_t channel, float32_t gain)
	{
		if (channel >= NN) return;

		if (gain > MAX_GAIN)
			gain = MAX_GAIN;
		else if (gain < MIN_GAIN)
			gain = MIN_GAIN;
		multiplier[channel] = gain;
	}

	void gain(float32_t gain)
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

	void getMix(float32_t* buffer)
	{
		assert(buffer);
		assert(sumbufL);
		arm_copy_f32(sumbufL, buffer, buffer_length);

		if(sumbufL)
			arm_fill_f32(0.0f, sumbufL, buffer_length);
	}

protected:
	float32_t multiplier[NN];
	float32_t* sumbufL;
	uint16_t buffer_length;
};

template <int NN> class AudioStereoMixer : public AudioMixer<NN>
{
public:
	AudioStereoMixer(uint16_t len) : AudioMixer<NN>(len)
	{
		for (uint8_t i=0; i<NN; i++)
			panorama[i] = UNITY_PANORAMA;

		sumbufR=new float32_t[buffer_length];
		arm_fill_f32(0.0f, sumbufR, buffer_length);
	}

	~AudioStereoMixer()
	{
		delete [] sumbufR;
	}

        void pan(uint8_t channel, float32_t pan)
	{
		if (channel >= NN) return;

		if (pan > MAX_PANORAMA)
			pan = MAX_PANORAMA;
		else if (pan < MIN_PANORAMA)
			pan = MIN_PANORAMA;
		panorama[channel] = pan;
	}

	void doAddMix(uint8_t channel, float32_t* in)
	{
		float32_t tmp[buffer_length];

		assert(in);

		// left
		arm_scale_f32(in, 1.0f-panorama[channel], tmp, buffer_length);
		if(multiplier[channel]!=UNITY_GAIN)
			arm_scale_f32(tmp,multiplier[channel],tmp,buffer_length);
		arm_add_f32(sumbufL, tmp, sumbufL, buffer_length);
		// right
		arm_scale_f32(in, panorama[channel], tmp, buffer_length);
		if(multiplier[channel]!=UNITY_GAIN)
			arm_scale_f32(tmp,multiplier[channel],tmp,buffer_length);
		arm_add_f32(sumbufR, tmp, sumbufR, buffer_length);
	}

	void doAddMix(uint8_t channel, float32_t* inL, float32_t* inR)
	{
		float32_t tmp[buffer_length];

		assert(inL);
		assert(inR);

		// left
		if(multiplier[channel]!=UNITY_GAIN)
			arm_scale_f32(inL,multiplier[channel],tmp,buffer_length);
		arm_add_f32(sumbufL, tmp, sumbufL, buffer_length);
		// right
		if(multiplier[channel]!=UNITY_GAIN)
			arm_scale_f32(inR,multiplier[channel],tmp,buffer_length);
		arm_add_f32(sumbufR, tmp, sumbufR, buffer_length);
	}

	void getMix(float32_t* bufferL, float32_t* bufferR)
	{
		assert(bufferR);
		assert(bufferL);
		assert(sumbufL);
		assert(sumbufR);

		arm_copy_f32 (sumbufL, bufferL, buffer_length);
		arm_copy_f32 (sumbufR, bufferR, buffer_length);

		if(sumbufL)
			arm_fill_f32(0.0f, sumbufL, buffer_length);
		if(sumbufR)
			arm_fill_f32(0.0f, sumbufR, buffer_length);
	}

protected:
	using AudioMixer<NN>::sumbufL;
	using AudioMixer<NN>::multiplier;
	using AudioMixer<NN>::buffer_length;
	float32_t panorama[NN];
	float32_t* sumbufR;
};

#endif
