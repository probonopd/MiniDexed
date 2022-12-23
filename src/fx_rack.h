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
#include "fx_tape_delay.h"
#include "fx_shimmer_reverb.h"

#include <vector>

class FXUnit : public FX
{
    DISALLOW_COPY_AND_ASSIGN(FXUnit);

public:
    FXUnit(float32_t sampling_rate, FX& fx, float32_t wet_level = 0.5f);
    virtual ~FXUnit();

    virtual void process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples) override;

    void setWetLevel(float32_t wet_level);
    inline float32_t getWetLevel() const;

private:
    FX& fx_;          // Embedded FX
    float32_t wet_level_;   // How much the signal is affected by the inner FX (0.0 - 1.0)
};

typedef std::vector<FXUnit*> FXChain;

class FXRack : public FX
{
    DISALLOW_COPY_AND_ASSIGN(FXRack);

public:
    FXRack(float32_t sampling_rate);
    virtual ~FXRack();

    virtual void process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples) override;

private:
    void registerFX(FX* fx);

    FXChain fx_chain_;
};