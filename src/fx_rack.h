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
// fx_rack.h
//
// Rack of audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx.h"
#include "fx_unit.hpp"
#include "fx_tube.h"
#include "fx_chorus.h"
#include "fx_flanger.h"
#include "fx_orbitone.h"
#include "fx_phaser.h"
#include "fx_delay.h"
#include "fx_shimmer_reverb.h"

#include <vector>

typedef std::vector<FXElement*> FXChain;

class FXRack : virtual public FX, virtual public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(FXRack);

public:
    FXRack(float32_t sampling_rate, bool enable = true, float32_t wet = 1.0f);
    virtual ~FXRack();

    virtual inline void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;
    virtual void process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples) override;

    void setEnable(bool enable = true);
    bool isEnable() const;

    void setWetLevel(float32_t wet_level);
    float32_t getWetLevel() const;

    FXUnit<Tube>* getTube();
    FXUnit<Chorus>* getChorus();
    FXUnit<Flanger>* getFlanger();
    FXUnit<Orbitone>* getOrbitone();
    FXUnit<Phaser>* getPhaser();
    FXUnit<Delay>* getDelay();
    FXUnit<ShimmerReverb>* getShimmerReverb();

private:
    void registerFX(FXElement* fx);

    bool enable_;
    float32_t wet_level_;

    FXChain fx_chain_;
    FXUnit<Tube>* fxTube_;
    FXUnit<Chorus>* fxChorus_;
    FXUnit<Flanger>* fxFlanger_;
    FXUnit<Orbitone>* fxOrbitone_;
    FXUnit<Phaser>* fxPhaser_;
    FXUnit<Delay>* fxDelay_;
    FXUnit<ShimmerReverb>* fxShimmerReverb_;
};