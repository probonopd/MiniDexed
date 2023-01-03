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
// fx_shimmer_reverb.h
//
// Stereo Shimmer reverb proposed in the context of the MiniDexed project
//
#pragma once

#include "fx_components.h"

class ShimmerReverb : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(ShimmerReverb);

public:
    ShimmerReverb(  float32_t sampling_rate, 
                    float32_t left_delay_time = 0.5f, 
                    float32_t right_delay_time = 0.6f, 
                    float32_t shimmer_frequency = 2.0f,
                    float32_t shimmer_amplitude = 0.5f,
                    float32_t decay_time = 2.0f);

    virtual ~ShimmerReverb();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setLeftDelayTime(float32_t delay_time_L);
    float32_t getLeftDelayTime() const;

    void setRightDelayTime(float32_t delay_time_R);
    float32_t getRightDelayTime() const;

    void setShimmerFrequency(float32_t frequency);
    float32_t getShimmerFrequency() const;

    void setShimmerAmplitude(float32_t amplitude);
    float32_t getShimmerAmplitude() const;

    void setDecayTime(float32_t decay_time);
    float32_t getDecayTime() const;

private:
    const unsigned DelayLineLength;
    float32_t* delay_line_L_;
    float32_t* delay_line_R_;

    // Current write position for left and right channel delay lines
    unsigned write_pos_L_;
    unsigned write_pos_R_;
    float32_t shimmer_phase_;
    float32_t shimmer_phase_increment_;

    float32_t delay_time_L_;            // Left channel delay time in seconds
    float32_t delay_time_R_;            // Right channel delay time in seconds
    float32_t shimmer_frequency_;       // Shimmer frequency parameter in Hz (0.0 === 20Hz - 1.0 === 20kHz)
    float32_t shimmer_amplitude_;       // Shimmer amplitude (0.0 - 1.0)
    float32_t decay_time_;              // Reverb decay time in seconds
};