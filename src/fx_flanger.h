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
// fx_flanger.h
//
// Stereo Flanger audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx_components.h"

#define MAX_FLANGER_DELAY 2.0f

class Flanger : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Flanger);

public:
    enum LFOIndex
    {
        LFO_L = 0,
        LFO_R,
        kLFOCount
    };

    Flanger(float32_t sampling_rate, float32_t rate = 0.5f, float32_t depth = 0.5f, float32_t feedback = 0.0f);
    virtual ~Flanger();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setRate(float32_t rate);
    float32_t getRate() const;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

    void setFeedback(float32_t feedback);
    float32_t getFeedback() const;

private:
    const unsigned MaxDelayLineSize;
    float32_t* delay_lineL_;
    float32_t* delay_lineR_;
    unsigned write_index_;
    float32_t feedback_samples_[StereoChannels::kNumChannels];

    LFO* lfo_[LFOIndex::kLFOCount];
    float32_t depth_;           // Depth of the flanger effect in milliseconds (0.0 - 10.0)
    float32_t feedback_;        // Amount of feedback to apply to the delay line

    IMPLEMENT_DUMP(
        const size_t space = 22;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "write_index_");
        SS__TEXT(ss, ' ', space, std::left, '|', "feedback_samples_[ L ]");
        SS__TEXT(ss, ' ', space, std::left, '|', "feedback_samples_[ R ]");
        SS__TEXT(ss, ' ', space, std::left, '|', "depth_");
        SS__TEXT(ss, ' ', space, std::left, '|', "feedback_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->write_index_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->feedback_samples_[StereoChannels::Left ]);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->feedback_samples_[StereoChannels::Right]);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->depth_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->feedback_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            out << "Flanger internal delay lines:" << std::endl;

            SS_RESET(ss, precision, std::left);
            SS__TEXT(ss, ' ', space, std::left, '|', "index");
            SS__TEXT(ss, ' ', space, std::left, '|', "delay_lineL_");
            SS__TEXT(ss, ' ', space, std::left, '|', "delay_lineR_");
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            out << "\t" << ss.str() << std::endl;

            for(size_t i = 0; i < this->MaxDelayLineSize; ++i)
            {
                SS_RESET(ss, precision, std::left);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", i);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->delay_lineL_[i]);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->delay_lineR_[i]);
                out << "\t" << ss.str() << std::endl;
            }

            for(size_t i = 0; i < LFOIndex::kLFOCount; ++i)
            {
                this->lfo_[i]->dump(out, deepInspection, tag + ".lfo_[ " + std::to_string(i) + " ]");
            }
        }

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".write_index_", static_cast<float32_t>(this->write_index_), 0.0, static_cast<float32_t>(this->MaxDelayLineSize),  deepInspection);
        nb_errors += inspector(tag + ".feedback_samples_[ L ]", this->feedback_samples_[StereoChannels::Left ], -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".feedback_samples_[ R ]", this->feedback_samples_[StereoChannels::Right], -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".depth_", this->depth_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".feedback_", this->feedback_, 0.0f, 0.97f, deepInspection);

        if(deepInspection)
        {
            for(size_t i = 0; i < this->MaxDelayLineSize; ++i)
            {
                nb_errors += inspector(tag + ".delay_lineL_[ " + std::to_string(i) + " ]", this->delay_lineL_[i], -1.0f, 1.0f, deepInspection);
                nb_errors += inspector(tag + ".delay_lineR_[ " + std::to_string(i) + " ]", this->delay_lineR_[i], -1.0f, 1.0f, deepInspection);
            }

            for(size_t i = 0; i < LFOIndex::kLFOCount; ++i)
            {
                nb_errors += this->lfo_[i]->inspect(inspector, deepInspection, tag + ".lfo_[ " + std::to_string(i) + " ]");
            }
        }

        return nb_errors;
    )
};
