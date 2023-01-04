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
// fx_shimmer_reverb3.h
//
// Stereo Shimmer Reverb proposed in the context of the MiniDexed project 
// It is adapted from the Shimmer Reverb that could be found on Cloud EuroRack module from Mutable Instrruments
//
#pragma once

#include "fx_components.h"
#include "fx_engine.hpp"

class ShimmerReverb : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(ShimmerReverb);

public:
    ShimmerReverb(float32_t sampling_rate);

    virtual ~ShimmerReverb();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setInputGain(float32_t gain);
    float32_t getInputGain() const;

    void setTime(float32_t time);
    float32_t getTime() const;

    void setDiffusion(float32_t diffusion);
    float32_t getDiffusion() const;

    void setLP(float32_t lp);
    float32_t getLP() const;

private:
    typedef FxEngine<16384, FORMAT_16_BIT, true> Engine;
    Engine engine_;

    float32_t input_gain_;
    float32_t reverb_time_;
    float32_t diffusion_;
    float32_t lp_;

    float32_t lp_decay_1_;
    float32_t lp_decay_2_;
};
