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

#include "fx_components.h"

#define MAX_FLANGER_DELAY 2.0f

class Flanger : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Flanger);

public:
    enum LFOIndex
    {
        LFO_L = 0,
        LFO_R
    };

    Flanger(float32_t sampling_rate, float32_t rate = 0.5f, float32_t depth = 0.5f, float32_t feedback = 0.0f);
    virtual ~Flanger();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setRate(float32_t rate);
    float32_t getRate() const;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

    void setFeedback(float32_t feedback);
    float32_t getFeedback() const;

private:
    const unsigned MaxDelayLineSize;
    float32_t* delay_lineL_;
    float32_t* delay_lineR_;
    unsigned write_index_;
    float32_t feedback_samples_[2];

    LFO* lfo_[2];
    float32_t depth_;           // Depth of the flanger effect in milliseconds (0.0 - 10.0)
    float32_t feedback_;        // Amount of feedback to apply to the delay line
};