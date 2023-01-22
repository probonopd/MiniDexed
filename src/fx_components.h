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
//
#pragma once

#include "fx.h"

#include <algorithm>
#include <random>
#include <cassert>

struct Constants
{
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
    FastLFO(float32_t sampling_rate, float32_t min_frequency = 0.01f, float32_t max_frequency = 10.0f, float32_t initial_phase = 0.0f);
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
    float32_t       frequency_;
    float32_t       normalized_frequency_;
    float32_t       unitary_frequency_;

    float32_t       y0_;
    float32_t       y1_;
    float32_t       iir_coefficient_;
    float32_t       initial_amplitude_;
    float32_t       current_;
};


class InterpolatedSineOscillator : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(InterpolatedSineOscillator);

public:
    InterpolatedSineOscillator(float32_t sampling_rate, float32_t min_frequency = 0.01f, float32_t max_frequency = 10.0f, float32_t initial_phase = 0.0f);
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
    static const size_t DataPointSize = 192000;
    static const float32_t DeltaTime; 
    static float32_t DataPoints[];

    const float32_t                             InitialPhase;
    const float32_t                             min_frequency_;
    const float32_t                             max_frequency_;
    float32_t                                   frequency_;
    float32_t                                   normalized_frequency_;
    float32_t                                   phase_index_;
    float32_t                                   phase_index_increment_;
    float32_t                                   current_sample_;
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

    ComplexLFO(float32_t sampling_rate, float32_t min_frequency = 0.01f, float32_t max_frequency = 10.0f, float32_t initial_phase = 0.0f);
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
};


typedef InterpolatedSineOscillator LFO;


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
};


class PerlinNoiseGenerator : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(PerlinNoiseGenerator);

public:
    PerlinNoiseGenerator(float32_t sampling_rate, float32_t rate = 0.2f);
    virtual ~PerlinNoiseGenerator();

    void setRate(float32_t rate);
    float32_t getRate() const;

    float32_t getCurrent() const;

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
};

float32_t softSaturator1(float32_t in, float32_t threshold);
float32_t softSaturator2(float32_t in, float32_t saturation);
float32_t softSaturator3(float32_t in, float32_t saturation);
float32_t softSaturator4(float32_t in, float32_t saturation);

float32_t waveFolder(float32_t input, float32_t bias);