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
// fx_phaser.h
//
// Stereo Phaser audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx_components.h"

#define MAX_NB_PHASES 24

class Phaser : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Phaser);

public:
    class AllpassDelay : public FXElement
    {
        DISALLOW_COPY_AND_ASSIGN(AllpassDelay);

    public:
        AllpassDelay();
        virtual ~AllpassDelay();

        virtual void reset() override;
        virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

        void setDelay(float32_t delayL, float32_t delayR);

    private:
        float32_t a1_[StereoChannels::kNumChannels];
        float32_t z_[StereoChannels::kNumChannels];

        IMPLEMENT_DUMP(
            const size_t space = 10;
            const size_t precision = 6;

            std::stringstream ss;

            out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

            SS_RESET(ss, precision, std::left);
            SS__TEXT(ss, ' ', space, std::left, '|', "a1_");
            SS__TEXT(ss, ' ', space, std::left, '|', "z_[ L ]");
            SS__TEXT(ss, ' ', space, std::left, '|', "z_[ R ]");
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS__TEXT(ss, ' ', space - 1, std::right, " |", this->a1_);
            SS__TEXT(ss, ' ', space - 1, std::right, " |", this->z_[StereoChannels::Left ]);
            SS__TEXT(ss, ' ', space - 1, std::right, " |", this->z_[StereoChannels::Right]);
            out << "\t" << ss.str() << std::endl;

            out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
        )

        IMPLEMENT_INSPECT(
            return 0u;
        )
    };

    Phaser(float32_t sampling_rate, float32_t rate = 0.5f, float32_t depth = 1.0f, float32_t feedback = 0.7f, unsigned nb_stages = 12);
    virtual ~Phaser();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setFrequencyRange(float32_t min_frequency, float32_t max_frequency);

    void setRate(float32_t rate);
    float32_t getRate() const;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

    void setFeedback(float32_t depth);
    float32_t getFeedback() const;

    void setNbStages(unsigned nb_stages);
    unsigned getNbStages() const;

private:
    LFO* lfo_[StereoChannels::kNumChannels];
    float32_t depth_;
    float32_t gain_;
    float32_t feedback_;
    float32_t dmin_;
    float32_t dmax_;
    unsigned nb_stages_;
    AllpassDelay stages_[MAX_NB_PHASES];
    float32_t z_[StereoChannels::kNumChannels];

    IMPLEMENT_DUMP(
        const size_t space = 12;
        const size_t precision = 6;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "depth_");
        SS__TEXT(ss, ' ', space, std::left, '|', "feedback_");
        SS__TEXT(ss, ' ', space, std::left, '|', "dmin_");
        SS__TEXT(ss, ' ', space, std::left, '|', "dmax_");
        SS__TEXT(ss, ' ', space, std::left, '|', "nb_stages_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->depth_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->feedback_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->dmin_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->dmax_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->nb_stages_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            this->lfo_[StereoChannels::Left ]->dump(out, deepInspection, tag + ".lfo_[ L ]");
            this->lfo_[StereoChannels::Right]->dump(out, deepInspection, tag + ".lfo_[ R ]");
            for(unsigned i = 0; i < MAX_NB_PHASES; ++i)
            {
                this->stages_[i].dump(out, deepInspection, tag + ".stages_[ " + std::to_string(i) + " ]");
            }
        }

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".depth_", this->depth_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".feedback_", this->feedback_, 0.0f, 0.97f, deepInspection);
        nb_errors += inspector(tag + ".nb_stages_", static_cast<float32_t>(this->nb_stages_), 0.0f, static_cast<float32_t>(MAX_NB_PHASES), deepInspection);

        if(deepInspection)
        {
            nb_errors += this->lfo_[StereoChannels::Left ]->inspect(inspector, deepInspection, tag + ".lfo_[ L ]");
            nb_errors += this->lfo_[StereoChannels::Right]->inspect(inspector, deepInspection, tag + ".lfo_[ R ]");
            for(unsigned i = 0; i < MAX_NB_PHASES; ++i)
            {
                nb_errors += this->stages_[i].inspect(inspector, deepInspection, tag + ".stages_[ " + std::to_string(i) + " ]");
            }
        }

        return nb_errors;
    )
};