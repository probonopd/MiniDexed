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
// fx_flanger.h
//
// Stereo Flanger audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx.h"

class Flanger : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Flanger);

public:
    Flanger(float32_t sampling_rate, float32_t delay_time = 5.0f, float32_t frequency = 0.5f, float32_t depth = 1.0f, float32_t feedback = 0.5f);
    virtual ~Flanger();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setDelayTime(float32_t delayMS);
    inline float32_t getDelayTime() const;

    void setFrequency(float32_t frequency);
    inline float32_t getFrequency() const;

    void setDepth(float32_t depth);
    inline float32_t getDepth() const;

    void setFeedback(float32_t feedback);
    inline float32_t getFeedback() const;

private:
    inline void adjustDelayCofficients();

    const unsigned MaxDelayLineSize;
    unsigned delay_line_index_;
    unsigned delay_line_size_;
    float32_t* delay_lineL_;
    float32_t* delay_lineR_;

    float32_t lfo_phase_;
    float32_t lfo_phase_increment_;

    float32_t delay_time_ms_;   // Delay time in milliseconds
    float32_t frequency_;       // LFO frequency in HZ (0.1 - 10.0)
    float32_t depth_;           // Depth of the flanger effect in milliseconds
    float32_t feedback_;        // Amount of feedback to apply to the delay line
};