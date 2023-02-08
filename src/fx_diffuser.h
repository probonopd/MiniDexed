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
// Stereo Diffuser proposed in the context of the MiniDexed project 
// It is adapted from the Diffuser that could be found on Cloud EuroRack module from Mutable Instrruments
//
#pragma once

#include "fx_components.h"
#include "fx_engine.hpp"

#define DIFFUSER_BUFFER_SIZE 2048

class Diffuser : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Diffuser);

public:
    Diffuser(float32_t sampling_frequency);
    virtual ~Diffuser();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

private:
    typedef FxEngine<DIFFUSER_BUFFER_SIZE, Format::FORMAT_FLOAT32, false> Engine;
    Engine engine_;

    IMPLEMENT_DUMP(
        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        if(deepInspection)
        {
            this->engine_.dump(out, deepInspection, tag + ".engine_");
        }
    
        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        if(deepInspection)
        {
            nb_errors += this->engine_.inspect(inspector, deepInspection, tag + ".engine_");
        }

        return nb_errors;
    )
};
