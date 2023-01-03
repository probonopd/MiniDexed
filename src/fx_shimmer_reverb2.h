// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

//
// fx_shimmer_reverb2.h
//
// Stereo Shimmer reverb proposed in the context of the MiniDexed project
//
#pragma once

#include "fx_components.h"
#include "fx_unit.hpp"

#define SHIMMER_BUFFER_SIZE 1024

#define SHIMMER_MIN_DECAY 0.0f
#define SHIMMER_MAX_DECAY 10.0f

#define SHIMMER_MIN_PITCH_RATIO 0.5f
#define SHIMMER_MAX_PITCH_RATIO 2.0f

class ShimmerReverb2 : 
        public virtual FXElement, 
        public virtual FXUnitModule
{
    DISALLOW_COPY_AND_ASSIGN(ShimmerReverb2);

public:
    ShimmerReverb2( float32_t sampling_rate, 
                    float32_t decay = 3.0f, 
                    float32_t diffusion = 0.5f, 
                    float32_t pitch_shift = 2.0f);

    virtual ~ShimmerReverb2();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setDecay(float32_t delay_time_L);
    float32_t getDecay() const;

    void setDiffusion(float32_t delay_time_R);
    float32_t getDiffusion() const;

    void setPitchShift(float32_t frequency);
    float32_t getPitchShift() const;

private:
    float32_t reverb_buffer_L_[SHIMMER_BUFFER_SIZE];
    float32_t reverb_buffer_R_[SHIMMER_BUFFER_SIZE];
    unsigned reverb_buffer_index_;

    // Current write position for left and right channel delay lines
    float32_t decay_;              // Reverb decay time in seconds (0 - 10)
    float32_t diffusion_;          // The degree to which the reverb is spread out over time (0 - 1)
    float32_t pitch_shift_;        // Determines the pitch shift ratio applied to the reverb (0.5 - 2.0)
};
