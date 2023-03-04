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
    Tube(float32_t sampling_rate);
    virtual ~Tube();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setOverdrive(float32_t overdrive);
    float32_t getOverdrive() const;

private:
    float32_t overdrive_;
    float32_t saturator_factor_;
    float32_t gain_factor_;

    IMPLEMENT_DUMP(
        const size_t space = 17;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "overdrive_");
        SS__TEXT(ss, ' ', space, std::left, '|', "saturator_factor_");
        SS__TEXT(ss, ' ', space, std::left, '|', "gain_factor_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->overdrive_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->saturator_factor_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->gain_factor_);
        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0;

        nb_errors += inspector(tag + ".overdrive_", this->overdrive_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".saturator_factor_", this->saturator_factor_, 1.0f, 201.0f, deepInspection);
        nb_errors += inspector(tag + ".gain_factor_", this->gain_factor_, 0.0f, 4.0f, deepInspection);

        return nb_errors;
    )
};