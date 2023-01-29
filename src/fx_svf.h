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

#include "fx.h"

class StateVariableFilter : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(StateVariableFilter);

public:
    typedef enum
    {
        LPF,    // Low pass filter
        HPF,    // High pass filter
        BPF     // Band pass filter
    } Type;

    StateVariableFilter(float32_t sampling_rate, Type type, float32_t cutoff);
    virtual ~StateVariableFilter();

    void setFilterType(Type type);
    void setGainDB(float32_t gainDB);
    void setCutoff(float32_t cutoff);
    void setResonance(float32_t resonance);

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

private:
    void updateCoefficients();

    Type type_;
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
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
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