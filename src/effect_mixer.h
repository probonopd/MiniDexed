// Taken from https://github.com/manicken/Audio/tree/templateMixer
// Adapted for MiniDexed by Holger Wirtz <dcoredump@googlemail.com>

#ifndef template_mixer_h_
#define template_mixer_h_

#include "arm_math.h"
#include <stdint.h>

#define UNITY_GAIN 1.0f
#define MAX_GAIN 1.0f
#define MIN_GAIN 0.0f
#define UNITY_PANORAMA 1.0f
#define MAX_PANORAMA 1.0f
#define MIN_PANORAMA 0.0f

template <int NN> class AudioMixer
{
public:
	AudioMixer(uint16_t len);
        void doAddMix(uint8_t channel, float32_t* in);
	void gain(uint8_t channel, float32_t gain);
	void gain(float32_t gain);
	void getMix(float32_t* buffer);
protected:
	float32_t multiplier[NN];
	float32_t* sumbufL;
	uint16_t buffer_length;
};

template <int NN> class AudioStereoMixer : public AudioMixer<NN>
{
public:
	AudioStereoMixer(uint16_t len);
        void pan(uint8_t channel, float32_t pan);
	void doAddMix(uint8_t channel, float32_t* in);
	void doAddMix(uint8_t channel, float32_t* inL, float32_t* inR);
	void getMix(float32_t* bufferL, float32_t* bufferR);
protected:
	using AudioMixer<NN>::sumbufL;
	using AudioMixer<NN>::multiplier;
	using AudioMixer<NN>::buffer_length;
	float32_t panorama[NN];
	float32_t* sumbufR;
};

#endif
