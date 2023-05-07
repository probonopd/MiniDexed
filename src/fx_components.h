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
// fx_components.h
//
// Several tools and components used in the implemlentation of FX
// Quthor: Vincent Gauché
//
#pragma once

#include "fx.h"

#include <algorithm>
#include <random>
#include <cassert>

#define LFO_MIN_FREQUENCY 0.01f
#define LFO_MAX_FREQUENCY 10.0f

struct Constants
{
    const static float32_t M_PI_POW_2;  // PI^2
    const static float32_t M_PI_POW_3;  // PI^3
    const static float32_t M_PI_POW_5;  // PI^5
    const static float32_t M_PI_POW_7;  // PI^7
    const static float32_t M_PI_POW_9;  // PI^9
    const static float32_t M_PI_POW_11; // PI^11

    const static float32_t M2PI;   // 2 * PI
    const static float32_t MPI_2;  // PI / 2
    const static float32_t MPI_3;  // PI / 3
    const static float32_t MPI_4;  // PI / 4
    const static float32_t M1_PI;  // 1 / PI
};


class FastLFO : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(FastLFO);

public:
    FastLFO(float32_t sampling_rate, float32_t min_frequency = LFO_MIN_FREQUENCY, float32_t max_frequency = LFO_MAX_FREQUENCY, float32_t initial_phase = 0.0f, bool centered = true);
    virtual ~FastLFO();

    void setNormalizedFrequency(float32_t normalized_frequency);
    float32_t getNormalizedFrequency() const;

    void setFrequency(float32_t frequency);
    float32_t getFrequency() const;

    virtual void reset() override;
    float32_t process();
    float32_t current() const;

private:
    void updateCoefficient();

    const float32_t InitialPhase;
    const float32_t min_frequency_;
    const float32_t max_frequency_;
    const bool      centered_;
    float32_t       frequency_;
    float32_t       normalized_frequency_;
    float32_t       unitary_frequency_;
    size_t          nb_sub_increment_;
    size_t          sub_increment_;

    float32_t       y0_;
    float32_t       y1_;
    float32_t       iir_coefficient_;
    float32_t       initial_amplitude_;
    float32_t       current_;

    IMPLEMENT_DUMP(
        const size_t space = 21;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "InitialPhase");
        SS__TEXT(ss, ' ', space, std::left, '|', "frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "normalized_frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "unitary_frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "y0_");
        SS__TEXT(ss, ' ', space, std::left, '|', "y1_");
        SS__TEXT(ss, ' ', space, std::left, '|', "iir_coefficient_");
        SS__TEXT(ss, ' ', space, std::left, '|', "initial_amplitude_");
        SS__TEXT(ss, ' ', space, std::left, '|', "current_");
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
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->InitialPhase);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->normalized_frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->unitary_frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->y0_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->y1_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->iir_coefficient_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->initial_amplitude_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->current_);

        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".InitialPhase", this->InitialPhase, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".frequency_", this->frequency_, this->min_frequency_, this->max_frequency_, deepInspection);
        nb_errors += inspector(tag + ".normalized_frequency_", this->normalized_frequency_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".unitary_frequency_", this->unitary_frequency_, this->min_frequency_ / this->getSamplingRate(), this->max_frequency_ / this->getSamplingRate(), deepInspection);
        nb_errors += inspector(tag + ".current_", this->current_, -1.0f, 1.0f, deepInspection);

        return nb_errors;
    )
};


class FastLFO2 : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(FastLFO2);

public:
    FastLFO2(float32_t sampling_rate, float32_t min_frequency = LFO_MIN_FREQUENCY, float32_t max_frequency = LFO_MAX_FREQUENCY, float32_t initial_phase = 0.0f, bool centered = true);
    virtual ~FastLFO2();

    void setNormalizedFrequency(float32_t normalized_frequency);
    float32_t getNormalizedFrequency() const;

    void setFrequency(float32_t frequency);
    float32_t getFrequency() const;

    virtual void reset() override;
    float32_t process();
    float32_t current() const;

private:
    const float32_t InitialPhase;
    const float32_t min_frequency_;
    const float32_t max_frequency_;
    const bool      centered_;
    float32_t       frequency_;
    float32_t       normalized_frequency_;
    float32_t       phase_;
    float32_t       phase_increment_;
    float32_t       current_;

    IMPLEMENT_DUMP(
        const size_t space = 21;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "InitialPhase");
        SS__TEXT(ss, ' ', space, std::left, '|', "frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "normalized_frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_increment_");
        SS__TEXT(ss, ' ', space, std::left, '|', "current_");
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
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->InitialPhase);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->normalized_frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_increment_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->current_);

        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".InitialPhase", this->InitialPhase, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".frequency_", this->frequency_, this->min_frequency_, this->max_frequency_, deepInspection);
        nb_errors += inspector(tag + ".normalized_frequency_", this->normalized_frequency_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".phase_", this->phase_, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".phase_", this->phase_increment_, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".current_", this->current_, -1.0f, 1.0f, deepInspection);

        return nb_errors;
    )
};


class InterpolatedSineOscillator : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(InterpolatedSineOscillator);

public:
    static float32_t Sin(float32_t phase);
    static float32_t Cos(float32_t phase);

    InterpolatedSineOscillator(float32_t sampling_rate, float32_t min_frequency = LFO_MIN_FREQUENCY, float32_t max_frequency = LFO_MAX_FREQUENCY, float32_t initial_phase = 0.0f, bool centered = true);
    virtual ~InterpolatedSineOscillator();

    void setNormalizedFrequency(float32_t normalized_frequency);
    float32_t getNormalizedFrequency() const;

    void setFrequency(float32_t frequency);
    float32_t getFrequency() const;

    virtual void reset() override;
    float32_t process();
    float32_t current() const;

private:
    static bool ClassInitializer();
    static const size_t DataPointSize = 176400;
    static const float32_t DeltaTime; 
    static float32_t CenteredDataPoints[];
    static float32_t UpliftDataPoints[];

    const float32_t InitialPhase;
    const float32_t min_frequency_;
    const float32_t max_frequency_;
    const bool      centered_;
    float32_t       frequency_;
    float32_t       normalized_frequency_;
    float32_t       phase_index_;
    float32_t       phase_index_increment_;
    float32_t       current_sample_;

    IMPLEMENT_DUMP(
        const size_t space = 22;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "InitialPhase");
        SS__TEXT(ss, ' ', space, std::left, '|', "normalized_frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_index_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_index_increment_");
        SS__TEXT(ss, ' ', space, std::left, '|', "current_sample_");
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
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->InitialPhase);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->normalized_frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_index_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_index_increment_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->current_sample_);
        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".InitialPhase", this->InitialPhase, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".normalized_frequency_", this->normalized_frequency_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".frequency_", this->frequency_, this->min_frequency_, this->max_frequency_, deepInspection);
        nb_errors += inspector(tag + ".phase_index_", this->phase_index_, 0.0f, static_cast<float32_t>(InterpolatedSineOscillator::DataPointSize), deepInspection);
        nb_errors += inspector(tag + ".current_sample_", this->current_sample_, -1.0f, 1.0f, deepInspection);

        return nb_errors;
    )
};

class ComplexLFO : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(ComplexLFO);

public:
    typedef enum {
        Sine,
        Saw,
        Square,
        SH,
        Noise
    } Waveform;

    ComplexLFO(float32_t sampling_rate, float32_t min_frequency = LFO_MIN_FREQUENCY, float32_t max_frequency = LFO_MAX_FREQUENCY, float32_t initial_phase = 0.0f, bool centered = true);
    virtual ~ComplexLFO();

    void setWaveform(Waveform waveform);
    Waveform getWaveform() const;

    void setNormalizedFrequency(float32_t normalized_frequency);
    float32_t getNormalizedFrequency() const;

    void setFrequency(float32_t frequency);
    float32_t getFrequency() const;

    virtual void reset() override;
    float32_t process();
    float32_t current() const;

private:
    const float32_t                             InitialPhase;
    const float32_t                             min_frequency_;
    const float32_t                             max_frequency_;
    const bool                                  centered_;
    Waveform                                    waveform_;
    float32_t                                   normalized_frequency_;
    float32_t                                   frequency_;
    float32_t                                   phase_;
    float32_t                                   phase_increment_;
    float32_t                                   current_sample_;
    bool                                        new_phase_;
    std::random_device                          rnd_device_;
    std::mt19937                                rnd_generator_;
    std::uniform_real_distribution<float32_t>   rnd_distribution_;

    IMPLEMENT_DUMP(
        const size_t space = 21;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "InitialPhase");
        SS__TEXT(ss, ' ', space, std::left, '|', "normalized_frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "frequency_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_increment_");
        SS__TEXT(ss, ' ', space, std::left, '|', "current_sample_");
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
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->InitialPhase);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->normalized_frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->frequency_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_increment_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->current_sample_);
        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".InitialPhase", this->InitialPhase, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".normalized_frequency_", this->normalized_frequency_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".frequency_", this->frequency_, this->min_frequency_, this->max_frequency_, deepInspection);
        nb_errors += inspector(tag + ".phase_", this->phase_, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".phase_increment_", this->phase_increment_, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".current_sample_", this->current_sample_, -1.0f, 1.0f, deepInspection);

        return nb_errors;
    )
};


typedef FastLFO2 LFO;


class JitterGenerator : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(JitterGenerator);

public:
    JitterGenerator(float32_t sampling_rate);
    virtual ~JitterGenerator();

    void setSpeed(float32_t speed);
    float32_t getSpeed() const;

    void setMagnitude(float32_t magnitude);
    float32_t getMagnitude() const;

    virtual void reset() override;
    float32_t process();

private:
    std::random_device                          rnd_device_;
    std::mt19937                                rnd_generator_;
    std::uniform_real_distribution<float32_t>   rnd_distribution_;
    float32_t speed_;
    float32_t magnitude_;
    float32_t phase_;
    float32_t phase_increment_;

    IMPLEMENT_DUMP(
        const size_t space = 16;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "speed_");
        SS__TEXT(ss, ' ', space, std::left, '|', "magnitude_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_increment_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->speed_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->magnitude_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_increment_);
        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".speed_", this->speed_, 0.0f, 0.45f * this->getSamplingRate(), deepInspection);
        nb_errors += inspector(tag + ".magnitude_", this->magnitude_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".phase_", this->phase_, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".phase_increment_", this->phase_increment_, 0.0f, 0.45f * Constants::M2PI, deepInspection);

        return nb_errors;
    )
};


class PerlinNoiseGenerator : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(PerlinNoiseGenerator);

public:
    PerlinNoiseGenerator(float32_t sampling_rate, float32_t rate = 0.2f);
    virtual ~PerlinNoiseGenerator();

    void setRate(float32_t rate);
    float32_t getRate() const;

    float32_t current() const;

    virtual void reset() override;
    float32_t process();

private:
    static int hash(int x);
    static float32_t interpolate(float32_t a, float32_t b, float32_t x);
    static float32_t perlin(float32_t x);

    float32_t rate_;
    float32_t phase_;
    float32_t phase_increment_;
    float32_t current_;

    static const float32_t Gradients[];

    IMPLEMENT_DUMP(
        const size_t space = 16;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "rate_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_");
        SS__TEXT(ss, ' ', space, std::left, '|', "phase_increment_");
        SS__TEXT(ss, ' ', space, std::left, '|', "current_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->rate_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->phase_increment_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->current_);
        out << "\t" << ss.str() << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".rate_", this->rate_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".phase_", this->phase_, 0.0f, Constants::M2PI, deepInspection);
        nb_errors += inspector(tag + ".phase_increment_", this->phase_increment_, 0.0f, Constants::M2PI / this->getSamplingRate(), deepInspection);
        nb_errors += inspector(tag + ".current_", this->current_, -1.0f, 1.0f, deepInspection);

        return nb_errors;
    )
};

float32_t softSaturator1(float32_t in, float32_t threshold);
float32_t softSaturator2(float32_t in, float32_t saturation);
float32_t softSaturator3(float32_t in, float32_t saturation);
float32_t softSaturator4(float32_t in, float32_t saturation);

float32_t waveFolder(float32_t input, float32_t bias);
