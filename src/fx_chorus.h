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
// fx_chorus.h
//
// Stereo Chorus audio effects proposed in the context of the MiniDexed project
// This implemelntation is based on the Chorus FX from the Rings Eurorack module from Mutable Instruments
// Ported by: Vincent Gauché
//
#pragma once

#include "fx_components.h"
#include "fx_engine.hpp"

#define CHORUS_FULLSCALE_DEPTH_RATIO 1536.0f

class Chorus : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Chorus);

public:
    enum LFOIndex
    {
        Sin1 = 0,
        Sin2,
        Cos1,
        Cos2,
        kLFOCount
    };

    Chorus(float32_t sampling_rate);
    virtual ~Chorus();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

    void setRate(float32_t rate);
    float32_t getRate() const;

private:
    typedef FxEngine<2048, Format::FORMAT_FLOAT32, false> Engine;
    Engine engine_;

    float32_t rate_;            // Normalized frequency for the 2 LFOs frequencies (0.0 - 10.0)
    float32_t depth_;           // Depth of the chorus in milliseconds (0.0 - 10.0)
    float32_t fullscale_depth_; // Equivalent to depth_ but in the range of (0.0 - 384.0)
    float32_t feedback_;        // Feedback level of the chorus (0.0 - 1.0)

    LFO* lfo_[LFOIndex::kLFOCount];

    IMPLEMENT_DUMP(
        const size_t space = 16;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        if(deepInspection)
        {
            this->engine_.dump(out, deepInspection, tag + ".engine_");
        }

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "rate_");
        SS__TEXT(ss, ' ', space, std::left, '|', "depth_");
        SS__TEXT(ss, ' ', space, std::left, '|', "fullscale_depth_");
        SS__TEXT(ss, ' ', space, std::left, '|', "feedback_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->rate_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->depth_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->fullscale_depth_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->feedback_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            for(size_t i = 0; i < LFOIndex::kLFOCount; ++i)
            {
                this->lfo_[i]->dump(out, deepInspection, tag + ".lfo_[ " + std::to_string(i) + " ]");
            }
        }

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        if(deepInspection)
        {
            nb_errors += this->engine_.inspect(inspector, deepInspection, tag + ".engine_");
            for(size_t i = 0; i < LFOIndex::kLFOCount; ++i)
            {
                nb_errors += this->lfo_[i]->inspect(inspector, deepInspection, tag + ".lfo_[ " + std::to_string(i) + " ]");
            }
        }
        nb_errors += inspector(tag + ".rate_", this->rate_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".depth_", this->depth_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".fullscale_depth_", this->fullscale_depth_, 0.0f, CHORUS_FULLSCALE_DEPTH_RATIO, deepInspection);

        return nb_errors;
    )
};
