#include "fx_components.h"

#include <cmath>

const float32_t Constants::M2PI = 2.0f * PI;
const float32_t Constants::M1_PI = 1.0f / PI;

LFO::LFO(float32_t sampling_rate, Waveform waveform, float32_t min_frequency, float32_t max_frequency) :
    FXBase(sampling_rate),
    min_frequency_(min_frequency),
    max_frequency_(max_frequency),
    phase_(0.0f),
    last_sample_(0.0f),
    new_phase_(true),
    rnd_generator_(rnd_device_()),
    rnd_distribution_(-1.0f, 1.0f)
{
    this->setWaveform(waveform);
    this->setFrequency(this->min_frequency_);
}

LFO::~LFO()
{
}

void LFO::setWaveform(Waveform waveform)
{
    this->waveform_ = waveform;
}

LFO::Waveform LFO::getWaveform() const
{
    return this->waveform_;
}

void LFO::setNormalizedFrequency(float32_t normalized_frequency)
{
    normalized_frequency = constrain(normalized_frequency, 0.0f, 1.0f);
    if(this->normalized_frequency_ != normalized_frequency)
    {
        float32_t frequency = mapfloat(normalized_frequency, 0.0f, 1.0f, this->min_frequency_, this->max_frequency_);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->phase_increment_ = Constants::M2PI * this->frequency_ / this->getSamplingRate();
    }
}

float32_t LFO::getNormalizedFrequency() const
{
    return this->frequency_;
}

void LFO::setFrequency(float32_t frequency)
{
    frequency = constrain(frequency, this->min_frequency_, this->max_frequency_);
    if(this->frequency_ != frequency)
    {
        float32_t normalized_frequency = mapfloat(frequency, this->min_frequency_, this->max_frequency_, 0.0f, 1.0f);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->phase_increment_ = Constants::M2PI * this->frequency_ / this->getSamplingRate();
    }
}

float32_t LFO::process()
{
    float32_t out = 0.0f;
    switch(this->waveform_)
    {
    case Waveform::Sine:
        out = std::sin(this->phase_);
        break;
    case Waveform::Saw:
        out = Constants::M1_PI * this->phase_ - 1.0f;
        break;
    case Waveform::Square:
        out = this->phase_ < PI ? 1.0 : -1.0;
        break;
    case Waveform::SH:
        if(this->new_phase_)
        {
            out = this->rnd_distribution_(this->rnd_generator_);
        }
        else
        {
            out = this->last_sample_;
        }
        break;
    case Waveform::Noise:
        out = this->rnd_distribution_(this->rnd_generator_);
        break;
    }

    this->last_sample_ = out;

    this->phase_ += this->phase_increment_;
    if(this->phase_ >= Constants::M2PI)
    {
        this->phase_ -= Constants::M2PI;
        this->new_phase_ = true;
    }
    else
    {
        this->new_phase_ = false;
    }

    return out;
}
