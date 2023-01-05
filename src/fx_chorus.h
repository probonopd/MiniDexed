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
// fx_chorus.h
//
// Stereo Chorus audio effects proposed in the context of the MiniDexed project
// This implemelntation is based on the Chorus FX from the Rings Eurorack module from Mutable Instruments
//
#pragma once

#include "fx_components.h"
#include "fx_engine.hpp"

class Chorus : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Chorus);

public:
    enum LFOIndex
    {
        Sin1 = 0,
        Sin2,
        Cos1,
        Cos2
    };

    Chorus(float32_t sampling_rate);
    virtual ~Chorus();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

    void setRate(float32_t rate);
    float32_t getRate() const;

private:
    typedef FxEngine<2048, FORMAT_16_BIT, false> Engine;
    Engine engine_;

    float32_t rate_;            // Normalized frequency for the 2 LFOs frequencies (0.0 - 10.0)
    float32_t depth_;           // Depth of the chorus in milliseconds (0.0 - 10.0)
    float32_t fullscale_depth_; // Equivalent to depth_ but in the range of (0.0 - 384.0)
    float32_t feedback_;        // Feedback level of the chorus (0.0 - 1.0)

    LFO* lfo_[4];
};
