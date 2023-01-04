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

    LFO(float32_t sampling_rate, Waveform waveform = Waveform::Sine, float32_t min_frequency = 0.01f, float32_t max_frequency = 10.0f, float32_t initial_phase = 0.0f);
    ~LFO();

    void setWaveform(Waveform waveform);
    Waveform getWaveform() const;

    void setNormalizedFrequency(float32_t normalized_frequency);
    float32_t getNormalizedFrequency() const;

    void setFrequency(float32_t frequency);
    float32_t getFrequency() const;

    float32_t process();
    float32_t current() const;

private:
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

template<typename T, unsigned size, unsigned nb_channels = 2, bool circular_buffer = true>
class Buffer
{
    DISALLOW_COPY_AND_ASSIGN(Buffer);

public:
    Buffer() : 
        index_(0)
    {
        this->values_ = new T*[nb_channels];
        for(unsigned i = 0; i < nb_channels; ++i)
        {
            this->values_[i] = new T[size];
        }
        this->reset();
    }

    virtual ~Buffer()
    {
        for(unsigned i = 0; i < nb_channels; ++i)
        {
            delete[] this->values_[i];
        }
        delete[] this->values_;
    }

    void reset(bool reset_index = true)
    {
        this->zero();

        if(reset_index)
        {
            this->index_ = 0;
        }
    }

    T& operator[](unsigned channel)
    {
        assert(channel < nb_channels);
        return *(this->values_[channel] + this->index_);
    }

    bool operator++()
    {
        this->index_++;
        if(this->index_ >= size)
        {
            if(circular_buffer)
            {
                this->index_ = 0;
                return true;
            }
            else
            {
                this->index_ = size - 1;
                return false;
            }
        }
        return true;
    }

    bool operator--()
    {
        if(this->index_ > 0)
        {
            this->index_--;
            return true;
        }
        else
        {
            if(circular_buffer)
            {
                this->index_ = size - 1;
                return true;
            }
            else
            {
                this->index_ = 0;
                return false;
            }
        }
    }

    void copy(T* buffer, unsigned channel, unsigned nb, bool from_start = true)
    {
        assert(channel < nb_channels);
        unsigned start = from_start ? 0 : this->index_;
        unsigned _nb = std::min(nb, size - start);
        memcpy(this->values_[channel] + start, buffer, _nb);
    }

    void zero()
    {
        for(unsigned c = 0; c < nb_channels; ++c)
        {
            memset(this->values_[c], 0, size * sizeof(T));
        }
    }

    void scale(T scale)
    {
        for(unsigned c = 0; c < nb_channels; ++c)
        {
            for(unsigned i = 0; i < size; ++i)
            {
                this->values_[c][i] *= scale;
            }
        }
    }

    unsigned index() const
    {
        return this->index_;
    }

    unsigned nbChannels() const
    {
        return nb_channels;
    }

    unsigned bufferSize() const
    {
        return size;
    }

    bool isCircularBuffer() const
    {
        return circular_buffer;
    }

private:
    unsigned index_;
    T** values_;
};

template<unsigned size, unsigned nb_channels, bool circular_buffer>
class Buffer<float32_t, size, nb_channels, circular_buffer>
{
    void scale(float32_t scale)
    {
        for(unsigned c = 0; c < nb_channels; ++c)
        {
            arm_scale_f32(this->values_[c], scale, this->values_[c], size);
        }
    }

    void copy(float32_t* buffer, unsigned channel, unsigned nb, bool from_start = true)
    {
        assert(channel < nb_channels);
        unsigned start = from_start ? 0 : this->index_;
        unsigned _nb = std::min(nb, size - start);
        arm_copy_f32(buffer, this->values_[channel] + start, _nb);
    }

};


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

float32_t softSaturator1(float32_t in, float32_t threshold);
float32_t softSaturator2(float32_t in, float32_t saturation);

float32_t waveFolder(float32_t input, float32_t bias);