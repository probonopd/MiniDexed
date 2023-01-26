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
// fx_unit.h
//
// Unit of FX that handle enable and wet parameters
//
#pragma once

#include "fx_components.h"

class FXUnitModule
{
    DISALLOW_COPY_AND_ASSIGN(FXUnitModule);

public:
    FXUnitModule(bool enable = true, float32_t wet_level = 0.5f)
    {
        this->setEnable(enable);
        this->setWetLevel(wet_level);
    }

    virtual ~FXUnitModule()
    {
    }

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) = 0;

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

protected:
    bool enable_;
    float32_t wet_level_;   // How much the signal is affected by the inner FX (0.0 - 1.0)
};

template<typename _FXElement>
class FXUnit : public virtual FXUnitModule, public virtual _FXElement
{
    DISALLOW_COPY_AND_ASSIGN(FXUnit);

public:
    FXUnit(float32_t sampling_rate, bool enable = true, float32_t wet_level = 0.5f) :
        FXUnitModule(),
        _FXElement(sampling_rate),
        is_reset_(false)
    {
        this->setEnable(enable);
        this->setWetLevel(wet_level);
    }

    virtual ~FXUnit()
    {
    }

    void reset()
    {
        if(!this->is_reset_)
        {
            _FXElement::reset();
            this->is_reset_ = true;
        }
    }

    void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
    {
        if(!this->isEnable() || this->getWetLevel() == 0.0f)
        {
            this->reset();

            outL = inL;
            outR = inR;
        }
        else
        {
            this->is_reset_ = false;
            _FXElement::processSample(inL, inR, outL, outR);

            float32_t dry = 1.0f - this->getWetLevel();
            outL = this->getWetLevel() * outL + dry * inL;
            outR = this->getWetLevel() * outR + dry * inR;
        }
    }

private:
    bool is_reset_;
};