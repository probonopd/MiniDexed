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
// fx_rack.h
//
// Rack of audio effects proposed in the context of the MiniDexed project
// Author: Vincent Gauché
//
#pragma once

#include "fx.h"
#include "fx_tube.h"
#include "fx_chorus.h"
#include "fx_flanger.h"
#include "fx_orbitone.h"
#include "fx_phaser.h"
#include "fx_delay.h"
#include "fx_reverberator.h"
#include "fx_unit.hpp"

#include <vector>

typedef std::vector<FXElement*> FXChain;

class FXRack : virtual public FX, virtual public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(FXRack);

public:
    FXRack(float32_t sampling_rate, bool enable = true, float32_t wet = 1.0f);
    virtual ~FXRack();

    virtual void reset() override;
    virtual inline void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;
    virtual void process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples) override;

    void setEnable(bool enable = true);
    bool isEnable() const;

    void setWetLevel(float32_t wet_level);
    float32_t getWetLevel() const;

    FXUnit<Tube>* getTube();
    FXUnit<Chorus>* getChorus();
    FXUnit<Flanger>* getFlanger();
    FXUnit<Orbitone>* getOrbitone();
    FXUnit<Phaser>* getPhaser();
    FXUnit<Delay>* getDelay();
    FXUnit<Reverberator>* getReverberator();

private:
    void registerFX(FXElement* fx);

    bool enable_;
    float32_t wet_level_;

    FXChain fx_chain_;
    FXUnit<Tube>* fxTube_;
    FXUnit<Chorus>* fxChorus_;
    FXUnit<Flanger>* fxFlanger_;
    FXUnit<Orbitone>* fxOrbitone_;
    FXUnit<Phaser>* fxPhaser_;
    FXUnit<Delay>* fxDelay_;
    FXUnit<Reverberator>* fxReverberator_;

    IMPLEMENT_DUMP(
        const size_t space = 10;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "enable_");
        SS__TEXT(ss, ' ', space, std::left, '|', "wet_level_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->enable_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->wet_level_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            this->fxTube_->dump(out, deepInspection, tag + ".fxTube_");
            this->fxChorus_->dump(out, deepInspection, tag + ".fxChorus_");
            this->fxFlanger_->dump(out, deepInspection, tag + ".fxFlanger_");
            this->fxOrbitone_->dump(out, deepInspection, tag + ".fxOrbitone_");
            this->fxPhaser_->dump(out, deepInspection, tag + ".fxPhaser_");
            this->fxDelay_->dump(out, deepInspection, tag + ".fxDelay_");
            this->fxReverberator_->dump(out, deepInspection, tag + ".fxReverberator_");
        }

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0;

        nb_errors += inspector(tag + ".enable_", this->enable_, -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".wet_level_", this->wet_level_, -1.0f, 1.0f, deepInspection);

        if(deepInspection)
        {
            nb_errors += this->fxTube_->inspect(inspector, deepInspection, tag + ".fxTube_");
            nb_errors += this->fxChorus_->inspect(inspector, deepInspection, tag + ".fxChorus_");
            nb_errors += this->fxFlanger_->inspect(inspector, deepInspection, tag + ".fxFlanger_");
            nb_errors += this->fxOrbitone_->inspect(inspector, deepInspection, tag + ".fxOrbitone_");
            nb_errors += this->fxPhaser_->inspect(inspector, deepInspection, tag + ".fxPhaser_");
            nb_errors += this->fxDelay_->inspect(inspector, deepInspection, tag + ".fxDelay_");
            nb_errors += this->fxReverberator_->inspect(inspector, deepInspection, tag + ".fxReverberator_");
        }

        return nb_errors;
    )
};