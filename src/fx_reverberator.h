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
// fx_reverberator.h
//
// Stereo Reverberator proposed in the context of the MiniDexed project 
// It is adapted from the Reverb that could be found on Cloud EuroRack module from Mutable Instrruments
//
#pragma once

#include "fx_components.h"
#include "fx_engine.hpp"

#define REVERBERATOR_BUFFER_SIZE 16384

class Reverberator : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Reverberator);

public:
    Reverberator(float32_t sampling_rate);
    virtual ~Reverberator();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setInputGain(float32_t gain);
    float32_t getInputGain() const;

    void setTime(float32_t time);
    float32_t getTime() const;

    void setDiffusion(float32_t diffusion);
    float32_t getDiffusion() const;

    void setLP(float32_t lp);
    float32_t getLP() const;

private:
    typedef FxEngine<REVERBERATOR_BUFFER_SIZE, Format::FORMAT_FLOAT32, true> Engine;
    Engine engine_;

    float32_t input_gain_;
    float32_t reverb_time_;
    float32_t diffusion_;
    float32_t lp_;

    float32_t lp_decay_1_;
    float32_t lp_decay_2_;

    IMPLEMENT_DUMP(
        const size_t space = 12;
        const size_t precision = 6;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "input_gain_");
        SS__TEXT(ss, ' ', space, std::left, '|', "reverb_time_");
        SS__TEXT(ss, ' ', space, std::left, '|', "diffusion_");
        SS__TEXT(ss, ' ', space, std::left, '|', "lp_");
        SS__TEXT(ss, ' ', space, std::left, '|', "lp_decay_1_");
        SS__TEXT(ss, ' ', space, std::left, '|', "lp_decay_2_");
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
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->input_gain_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->reverb_time_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->diffusion_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->lp_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->lp_decay_1_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->lp_decay_2_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            this->engine_.dump(out, deepInspection, tag + ".engine_");
        }
    
        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".input_gain_", this->input_gain_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".reverb_time_", this->reverb_time_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".diffusion_", this->diffusion_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".lp_", this->lp_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".lp_decay_1_", this->lp_decay_1_, -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".lp_decay_2_", this->lp_decay_2_, -1.0f, 1.0f, deepInspection);

        if(deepInspection)
        {
            nb_errors += this->engine_.inspect(inspector, deepInspection, tag + ".engine_");
        }

        return nb_errors;
    )
};
