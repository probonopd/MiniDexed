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
 
#ifndef template_mixer_h_
#define template_mixer_h_

#include "arm_math.h"
#include <stdint.h>

#define UNITYGAIN 1.0f
#define MAX_GAIN 1.0f
#define MIN_GAIN 0.0f

template <int NN> class AudioMixer
{
public:
	AudioMixer(void)
        {
	    for (uint8_t i=0; i<NN; i++)
                multiplier[i] = UNITYGAIN;
	}	
        void doAddMix(uint8_t channel, float32_t* in, float32_t* out, uint16_t len);
	/**
	 * this sets the individual gains
	 * @param channel
	 * @param gain
	 */
	void gain(uint8_t channel, float32_t gain);
	/**
	 * set all channels to specified gain
	 * @param gain
	 */
	void gain(float32_t gain);

protected:
	float32_t multiplier[NN];
};

template <int NN> class AudioStereoMixer : public AudioMixer<NN>
{
public:
	AudioStereoMixer(void)
	{
		AudioMixer<NN>();
	    	for (uint8_t i=0; i<NN; i++)
                	panorama[i] = 0.0;
	}
        void doAddMix(uint8_t channel, float32_t* in[], float32_t* out[], uint16_t len);
protected:
	float32_t panorama[NN];
};

#endif
