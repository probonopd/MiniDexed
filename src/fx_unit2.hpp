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
// fx_unit2.h
//
// Unit of FX that handle the mute parameter
//
#pragma once

#include "fx_components.h"

#include <iostream>
using namespace std;

class FXUnitModule2
{
    DISALLOW_COPY_AND_ASSIGN(FXUnitModule2);

public:
    FXUnitModule2(bool mute = false)
    {
        this->setMute(mute);
    }

    virtual ~FXUnitModule2()
    {
    }

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) = 0;

    void setMute(bool mute = false)
    {
        this->mute_ = mute;
    }

    inline bool isMute() const
    {
        return this->mute_;
    }

protected:
    bool mute_;
};

template<typename _FXElement>
class FXUnit2 : public virtual FXUnitModule2, public virtual _FXElement
{
    DISALLOW_COPY_AND_ASSIGN(FXUnit2);

public:
    FXUnit2(float32_t sampling_rate, bool mute = false) :
        FXUnitModule2(mute),
        _FXElement(sampling_rate),
        is_reset_(false)
    {
        this->setMute(mute);
    }

    virtual ~FXUnit2()
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
        if(this->isMute())
        {
            this->reset();

            outL = 0.0f;
            outR = 0.0f;
        }
        else
        {
            this->is_reset_ = false;
            _FXElement::processSample(inL, inR, outL, outR);
        }
    }

private:
    bool is_reset_;
};
