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
// Stereo Pitch Shifter proposed in the context of the MiniDexed project 
// It is adapted from the Pitch Shifter that could be found on Cloud EuroRack module from Mutable Instrruments
//
#pragma once

#include "fx_components.h"
#include "fx_engine.hpp"

#define PITCH_SHIFTER_BUFFER_SIZE 4096

class PitchShifter : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(PitchShifter);

public:
    PitchShifter(float32_t sampling_rate);
    virtual ~PitchShifter();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setTranspose(float32_t transpose);
    float32_t getTranspose() const;

    void setSize(float32_t size);
    float32_t getSize() const;

private:
    typedef FxEngine<PITCH_SHIFTER_BUFFER_SIZE, Format::FORMAT_FLOAT32, false> Engine;
    Engine engine_;

    float32_t phase_;
    float32_t transpose_;
    float32_t ratio_;
    float32_t size_;
    float32_t sample_size_;
  
    IMPLEMENT_DUMP()
    IMPLEMENT_INSPECT(return 0u;)
};
