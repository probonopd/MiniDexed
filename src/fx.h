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
// fx.h
//
// Base classes for Stereo audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include <stdint.h>
#include <arm_math.h>
#include "common.h"

#include "debug.hpp"
#include "fx_base.h"

class INSPECTABLE(FXBase)
{
    DISALLOW_COPY_AND_ASSIGN(FXBase);
    
protected:
    FXBase(float32_t sampling_rate);

public:
    virtual ~FXBase();
    
    float32_t getSamplingRate() const;

    virtual void reset() = 0;

private:
    const float32_t SamplingRate;
};

class FXElement : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(FXElement);

protected:
    FXElement(float32_t sampling_rate, float32_t output_level_corrector = 1.0f);

    const float32_t OutputLevelCorrector;

public:
    virtual ~FXElement();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) = 0;
};

class FX : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(FX);
    
protected:
    FX(float32_t sampling_rate);

public:
    virtual ~FX();

    virtual void process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples) = 0;
};
