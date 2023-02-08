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
// fx_svf.h
//
// State Variable Filter used in Tape Delay
//
#pragma once

#include "fx.h"
#include "fx_components.h"

class StateVariableFilter : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(StateVariableFilter);

public:
    enum FilterMode
    {
        LPF,    // Low pass filter
        HPF,    // High pass filter
        BPF     // Band pass filter
    };

    StateVariableFilter(float32_t sampling_rate, FilterMode mode, float32_t cutoff);
    virtual ~StateVariableFilter();

    void setFilterMode(FilterMode mode);
    void setGainDB(float32_t gainDB);
    void setCutoff(float32_t cutoff);
    void setResonance(float32_t resonance);

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

private:
    void updateCoefficients();

    FilterMode mode_;
    float32_t gain_;
    float32_t cutoff_;
    float32_t resonance_;
    float32_t g_;
    float32_t w_;
    float32_t a_;
    float32_t b_;
    float32_t c1_;
    float32_t c2_;
    float32_t d0_;
    float32_t d1_;
    float32_t z1_[StereoChannels::kNumChannels];
    float32_t z2_[StereoChannels::kNumChannels];

    IMPLEMENT_DUMP(
        const size_t space = 12;
        const size_t precision = 6;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "mode_");
        SS__TEXT(ss, ' ', space, std::left, '|', "gain_");
        SS__TEXT(ss, ' ', space, std::left, '|', "cutoff_");
        SS__TEXT(ss, ' ', space, std::left, '|', "resonance_");
        SS__TEXT(ss, ' ', space, std::left, '|', "g_");
        SS__TEXT(ss, ' ', space, std::left, '|', "w_");
        SS__TEXT(ss, ' ', space, std::left, '|', "a_");
        SS__TEXT(ss, ' ', space, std::left, '|', "b_");
        SS__TEXT(ss, ' ', space, std::left, '|', "c1_");
        SS__TEXT(ss, ' ', space, std::left, '|', "c2_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->mode_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->gain_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->cutoff_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->resonance_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->g_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->w_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->a_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->b_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->c1_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->c2_);
        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + "gain_", this->gain_, -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + "cutoff_", this->cutoff_, 1.0f, this->getSamplingRate() / 2.0f, deepInspection);
        nb_errors += inspector(tag + "resonance_", this->resonance_, 0.005f, 1.0f, deepInspection);
        nb_errors += inspector(tag + "g_", this->g_, 0.0f, 16.0f, deepInspection);
        nb_errors += inspector(tag + "w_", this->w_, 0.0f, 13.0f, deepInspection);
        nb_errors += inspector(tag + "a_", this->a_, 0.0f, 2526.0f, deepInspection);
        nb_errors += inspector(tag + "b_", this->b_, 0.0f, 160.0f, deepInspection);
        nb_errors += inspector(tag + "c1_", this->c1_, 0.0f, 2.06f, deepInspection);
        nb_errors += inspector(tag + "c2_", this->c2_, 0.0f, 0.06f, deepInspection);
        
        return nb_errors;
    )
};

class SVF : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(SVF);

public:
    enum FrequencyApproximation
    {
        FrequencyExact,
        FrequencyAccurate,
        FrequencyFast,
        FrequencyDirty
    };

    enum FilterMode
    {
        SVF_LP,
        SVF_BP,
        SVF_BP_NORMALIZED,
        SVF_HP
    };

    SVF(float32_t sampling_frequency, FilterMode mode = FilterMode::SVF_LP) : 
        FXElement(sampling_frequency),
        Mode(mode),
        g_(0.0f),
        r_(0.0f),
        h_(0.0f)
    {
        this->reset();
    }

    virtual ~SVF()
    {
    }

    inline virtual void reset() override
    {
        memset(this->state1_, 0, StereoChannels::kNumChannels * sizeof(float32_t));
        memset(this->state2_, 0, StereoChannels::kNumChannels * sizeof(float32_t));
    }

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override
    {
        float32_t hp, bp, lp;

        {
            hp = (inL - this->r_ * this->state1_[StereoChannels::Left ] - this->g_ * this->state1_[StereoChannels::Left ] - this->state2_[StereoChannels::Left ]) * this->h_;
            bp = this->g_ * hp + this->state1_[StereoChannels::Left ];
            this->state1_[StereoChannels::Left ] = this->g_ * hp + bp;
            lp = this->g_ * bp + this->state2_[StereoChannels::Left ];
            this->state2_[StereoChannels::Left ] = this->g_ * bp + lp;

            switch(this->Mode)
            {
            case FilterMode::SVF_LP:
                outL = lp;
                break;

            case FilterMode::SVF_BP:
                outL = bp;
                break;

            case FilterMode::SVF_BP_NORMALIZED:
                outL = bp * this->r_;
                break;

            case FilterMode::SVF_HP:
                outL = hp;
                break;
            }
        }

        {
            hp = (inR - this->r_ * this->state1_[StereoChannels::Right] - this->g_ * this->state1_[StereoChannels::Right] - this->state2_[StereoChannels::Right]) * this->h_;
            bp = this->g_ * hp + this->state1_[StereoChannels::Right];
            this->state1_[StereoChannels::Right] = this->g_ * hp + bp;
            lp = this->g_ * bp + this->state2_[StereoChannels::Right];
            this->state2_[StereoChannels::Right] = this->g_ * bp + lp;

            switch(this->Mode)
            {
            case FilterMode::SVF_LP:
                outR = lp;
                break;

            case FilterMode::SVF_BP:
                outR = bp;
                break;

            case FilterMode::SVF_BP_NORMALIZED:
                outR = bp * this->r_;
                break;

            case FilterMode::SVF_HP:
                outR = hp;
                break;
            }
        }
    }

    inline void setGRH(float32_t g, float32_t r, float32_t h)
    {
        this->g_ = g;
        this->r_ = r;
        this->h_ = h;
    }

    inline void setGR(float32_t g, float32_t r)
    {
        this->g_ = g;
        this->r_ = r;
        this->h_ = 1.0f / (1.0f + this->r_ * this->g_ * this->g_ * this->g_);
    }

    template<FrequencyApproximation approximation>
    inline void setFQ(float32_t frequency, float32_t resonance)
    {
        this->g_ = SVF::tan<approximation>(frequency);
        this->r_ = 1.0f / resonance;
        this->h_ = 1.0f / (1.0f + this->r_ * this->g_ * this->g_ * this->g_);
    }

private:
    template<FrequencyApproximation approximation>
    static inline float32_t tan(float32_t f)
    {
        switch(approximation)
        {
        case FrequencyApproximation::FrequencyExact:
            {
                // Clip coefficient to about 100.
                f = constrain(f, 0.0f, 0.497f);
                return ::tan(PI * f);
            }

        case FrequencyApproximation::FrequencyDirty:
            {
                // Optimized for frequencies below 8kHz.
                const float32_t a = 3.736e-01 * Constants::M_PI_POW_3;
                return f * (PI + a * f * f);
            }

        case FrequencyApproximation::FrequencyFast:
            {
                // The usual tangent approximation uses 3.1755e-01 and 2.033e-01, but
                // the coefficients used here are optimized to minimize error for the
                // 16Hz to 16kHz range, with a sample rate of 48kHz.
                const float a = 3.260e-01 * Constants::M_PI_POW_3;
                const float b = 1.823e-01 * Constants::M_PI_POW_5;
                float f2 = f * f;
                return f * (PI + f2 * (a + b * f2));
            }
            
        case FrequencyApproximation::FrequencyAccurate:
            {
                // These coefficients don't need to be tweaked for the audio range.
                const float a = 3.333314036e-01 * Constants::M_PI_POW_3;
                const float b = 1.333923995e-01 * Constants::M_PI_POW_5;
                const float c = 5.33740603e-02  * Constants::M_PI_POW_7;
                const float d = 2.900525e-03    * Constants::M_PI_POW_9;
                const float e = 9.5168091e-03   * Constants::M_PI_POW_11;
                float f2 = f * f;
                return f * (PI + f2 * (a + f2 * (b + f2 * (c + f2 * (d + f2 * e)))));
            }
        }
    }

    const FilterMode Mode;
    float32_t g_;
    float32_t r_;
    float32_t h_;

    float32_t state1_[StereoChannels::kNumChannels];
    float32_t state2_[StereoChannels::kNumChannels];

    IMPLEMENT_DUMP(
        const size_t space = 12;
        const size_t precision = 6;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "g_");
        SS__TEXT(ss, ' ', space, std::left, '|', "r_");
        SS__TEXT(ss, ' ', space, std::left, '|', "h_");
        SS__TEXT(ss, ' ', space, std::left, '|', "state1_[ L ]");
        SS__TEXT(ss, ' ', space, std::left, '|', "state1_[ R ]");
        SS__TEXT(ss, ' ', space, std::left, '|', "state2_[ L ]");
        SS__TEXT(ss, ' ', space, std::left, '|', "state2_[ R ]");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->g_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->r_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->r_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->state1_[StereoChannels::Left ]);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->state1_[StereoChannels::Right]);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->state2_[StereoChannels::Left ]);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->state2_[StereoChannels::Right]);
        out << "\t" << ss.str() << std::endl;
    
        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".g_", this->g_, 0.0f, 106.11f, deepInspection);
        nb_errors += inspector(tag + ".r_", this->r_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".h_", this->h_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".state1_[ L ]", this->state1_[StereoChannels::Left ], -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".state1_[ R ]", this->state1_[StereoChannels::Right], -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".state2_[ L ]", this->state2_[StereoChannels::Left ], -1.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".state2_[ R ]", this->state2_[StereoChannels::Right], -1.0f, 1.0f, deepInspection);

        return nb_errors;
    )
};
