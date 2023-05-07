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
// fx_orbitone.h
//
// Stereo Orbitone audio effects proposed in the context of the MiniDexed project
// This audio effect is based on the Ensemble audio effect of the Rings Eurorack module by Mutable Instruments
// Ported by: Vincent Gauché
//
#pragma once

#include "fx_components.h"
#include "fx_engine.hpp"

#define ORBITONE_FULLSCALE_DEPTH_RATIO 256.0f

class Orbitone : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Orbitone);

public:
    enum LFOIndex
    {
        Slow0 = 0,
        Slow120,
        Slow240,
        Fast0,
        Fast120,
        Fast240,
        kLFOCount
    };

    Orbitone(float32_t sampling_rate, float32_t rate = 0.5f, float32_t depth = 0.5f);
    virtual ~Orbitone();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setRate(float32_t rate);
    float32_t getRate() const;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

private:
    typedef FxEngine<4096, Format::FORMAT_FLOAT32, false> Engine;
    Engine engine_;

    float32_t depth_;
    float32_t fullscale_depth_;

    LFO* lfo_[LFOIndex::kLFOCount];

    IMPLEMENT_DUMP(
        const size_t space = 16;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "depth_");
        SS__TEXT(ss, ' ', space, std::left, '|', "fullscale_depth_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->depth_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->fullscale_depth_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            this->engine_.dump(out, deepInspection, tag + ".engine_");

            for(size_t i = 0; i < LFOIndex::kLFOCount; ++i)
            {
                this->lfo_[i]->dump(out, deepInspection, tag + ".lfo_[ " + std::to_string(i) + " ]");
            }
        }

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".depth_", this->depth_, -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".fullscale_depth_", this->fullscale_depth_, 0.0f, ORBITONE_FULLSCALE_DEPTH_RATIO, deepInspection);

        if(deepInspection)
        {
            this->engine_.inspect(inspector, deepInspection, tag + ".engine_");

            for(size_t i = 0; i < LFOIndex::kLFOCount; ++i)
            {
                this->lfo_[i]->inspect(inspector, deepInspection, tag + ".lfo_[ " + std::to_string(i) + " ]");
            }
        }

        return nb_errors;
    )
};
