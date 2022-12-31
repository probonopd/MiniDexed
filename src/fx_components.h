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

#include <random>

struct Constants
{
    const static float32_t M2PI;   // 2 * PI
    const static float32_t M1_PI;  // 1 / PI
};

class LFO : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(LFO);

public:
    typedef enum {
        Sine,
        Saw,
        Square,
        SH,
        Noise
    } Waveform;

    LFO(float32_t sampling_rate, Waveform waveform = Waveform::Sine, float32_t min_frequency = 0.01f, float32_t max_frequency = 10.0f);
    ~LFO();

    void setWaveform(Waveform waveform);
    Waveform getWaveform() const;

    void setNormalizedFrequency(float32_t normalized_frequency);
    float32_t getNormalizedFrequency() const;

    void setFrequency(float32_t frequency);
    float32_t getFrequency() const;

    float32_t process();

private:
    const float32_t                             min_frequency_;
    const float32_t                             max_frequency_;
    Waveform                                    waveform_;
    float32_t                                   normalized_frequency_;
    float32_t                                   frequency_;
    float32_t                                   phase_;
    float32_t                                   phase_increment_;
    float32_t                                   last_sample_;
    bool                                        new_phase_;
    std::random_device                          rnd_device_;
    std::mt19937                                rnd_generator_;
    std::uniform_real_distribution<float32_t>   rnd_distribution_;
};

template<typename T>
class Buffer
{
    DISALLOW_COPY_AND_ASSIGN(Buffer);

public:
    Buffer(unsigned size) :
        size_(size)
    {
        this->values_ = new T[size];
        this->reset();
    }

    virtual ~Buffer()
    {
        delete[] this->values_;
    }

    void reset()
    {
        memset(this->values_, 0, this->size_ * sizeof(T));
    }

    float32_t& operator[](unsigned index)
    {
        return this->values_[index];
    }

    unsigned getSize() const
    {
        return this->size_;
    }

private:
    const unsigned size_;
    T* values_;
};
