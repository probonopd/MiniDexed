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
// fx_dry.h
//
// An FX that does nothing but used to generalize the processing.
// Author: Vincent Gauché
//
#pragma once

#include "fx_components.h"

class Dry : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Dry);

public:
    Dry(float32_t sampling_rate);
    virtual ~Dry();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    IMPLEMENT_DUMP(
        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        return 0u;
    )
};