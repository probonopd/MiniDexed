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
#define PITCH_SHIFTER_TRANSPOSE_BOUNDARY 36.0f

class PitchShifter : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(PitchShifter);

public:
    PitchShifter(float32_t sampling_rate, float32_t transpose_boundary = PITCH_SHIFTER_TRANSPOSE_BOUNDARY);
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

    const float32_t TransposeBoundary;
    float32_t phase_;
    float32_t transpose_;
    float32_t ratio_;
    float32_t size_;
    float32_t sample_size_;
  
    IMPLEMENT_DUMP(
        const size_t space = 12;
        const size_t precision = 6;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_");
        SS__TEXT(ss, ' ', space, std::left, '|', "transpose_");
        SS__TEXT(ss, ' ', space, std::left, '|', "ratio_");
        SS__TEXT(ss, ' ', space, std::left, '|', "size_");
        SS__TEXT(ss, ' ', space, std::left, '|', "sample_size_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->transpose_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->ratio_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->size_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->sample_size_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            this->engine_.dump(out, deepInspection, tag + ".engine_");
        }
    
        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;        
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".phase_", this->phase_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".transpose_", this->transpose_, -PITCH_SHIFTER_TRANSPOSE_BOUNDARY, PITCH_SHIFTER_TRANSPOSE_BOUNDARY, deepInspection);
        nb_errors += inspector(tag + ".ratio_", this->ratio_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".size_", this->size_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".sample_size_", this->sample_size_, 0.0f, 1.0f, deepInspection);

        if(deepInspection)
        {
            nb_errors += this->engine_.inspect(inspector, deepInspection, tag + ".engine_");
        }

        return nb_errors;
    )
};
