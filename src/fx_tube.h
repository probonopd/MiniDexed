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
// fx_tube.h
//
// Stereo Tube overdrive audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx.h"

class Tube : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Tube);

public:
    Tube(float32_t sampling_rate, float32_t curve = 2.0f, float32_t bias = 0.7f);
    virtual ~Tube();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setOverdrive(float32_t overdrive);
    inline float32_t getOverdrive() const;

private:
    const float32_t TubeCurve;
    const float32_t TubeBias;

    float32_t overdrive_;
};