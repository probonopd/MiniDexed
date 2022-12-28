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
#include "fx_tube.h"
#include "fx_chorus.h"
#include "fx_flanger.h"
#include "fx_orbitone.h"
#include "fx_phaser.h"
#include "fx_tape_delay.h"
#include "fx_shimmer_reverb.h"

#include <vector>

template<typename _FXElement>
class FXUnit : public virtual _FXElement
{
    DISALLOW_COPY_AND_ASSIGN(FXUnit);

public:
    FXUnit(float32_t sampling_rate, bool enable = true, float32_t wet_level = 0.5f) :
        _FXElement(sampling_rate)
    {
        this->setEnable(enable);
        this->setWetLevel(wet_level);
    }

    virtual ~FXUnit()
    {
    }

    void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
    {
        if(!this->isEnable() || this->getWetLevel() == 0.0f)
        {
            outL = inL;
            outR = inR;
        }
        else
        {
            _FXElement::processSample(inL, inR, outL, outR);
            float32_t dry = 1.0f - this->getWetLevel();
            outL = this->getWetLevel() * outL + dry * inL;
            outR = this->getWetLevel() * outR + dry * inR;
        }
    }

    void setEnable(bool enable = true)
    {
        this->enable_ = enable;
    }

    inline bool isEnable() const
    {
        return this->enable_;
    }

    void setWetLevel(float32_t wet_level)
    {
        this->wet_level_ = constrain(wet_level, 0.0f, 1.0f);
    }

    inline float32_t getWetLevel() const
    {
        return this->wet_level_;
    }

private:
    bool enable_;
    float32_t wet_level_;   // How much the signal is affected by the inner FX (0.0 - 1.0)
};

typedef std::vector<FXElement*> FXChain;

class FXRack : virtual public FX, virtual public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(FXRack);

public:
    FXRack(float32_t sampling_rate, bool enable = true);
    virtual ~FXRack();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;
    virtual void process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples) override;

    void setEnable(bool enable = true);
    bool isEnable() const;

    FXUnit<Tube>* getTube();
    FXUnit<Chorus>* getChorus();
    FXUnit<Flanger>* getFlanger();
    FXUnit<Orbitone>* getOrbitone();
    FXUnit<Phaser>* getPhaser();
    FXUnit<TapeDelay>* getTapeDelay();
    FXUnit<ShimmerReverb>* getShimmerReverb();

private:
    void registerFX(FXElement* fx);

    bool enable_;

    FXChain fx_chain_;
    FXUnit<Tube>* fxTube_;
    FXUnit<Chorus>* fxChorus_;
    FXUnit<Flanger>* fxFlanger_;
    FXUnit<Orbitone>* fxOrbitone_;
    FXUnit<Phaser>* fxPhaser_;
    FXUnit<TapeDelay>* fxTapeDelay_;
    FXUnit<ShimmerReverb>* fxShimmerReverb_;
};