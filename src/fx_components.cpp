#include "fx_components.h"

#include <cmath>

///////////////////////////////
// Constants implemlentation //
///////////////////////////////
const float32_t Constants::M2PI = 2.0f * PI;
const float32_t Constants::M1_PI = 1.0f / PI;

/////////////////////////
// LFO implemlentation //
/////////////////////////
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
    return this->normalized_frequency_;
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

float32_t LFO::getFrequency() const
{
    return this->frequency_;
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

////////////////////////////////////
// JitterGenerator implementation //
////////////////////////////////////
JitterGenerator::JitterGenerator(float32_t sampling_rate) : 
    FXBase(sampling_rate),
    rnd_generator_(rnd_device_()),
    rnd_distribution_(-1.0f, 1.0f),
    phase_(0.0f),
    phase_increment_(0.0f)
{
    this->setSpeed(1.0f);
    this->setMagnitude(0.1f);
}

JitterGenerator::~JitterGenerator()
{
}

void JitterGenerator::setSpeed(float32_t speed)
{
    if(this->speed_ != speed)
    {
        this->speed_ = speed;
        this->phase_increment_ = Constants::M2PI * this->speed_ / this->getSamplingRate();
    }
}

float32_t JitterGenerator::getSpeed() const
{
    return this->speed_;
}

void JitterGenerator::setMagnitude(float32_t magnitude)
{
    this->magnitude_ = magnitude;
}

float32_t JitterGenerator::getMagnitude() const
{
    return this->magnitude_;
}

float32_t JitterGenerator::process()
{
    float32_t out = std::sin(this->phase_);

    this->phase_ += this->phase_increment_ * (1.0f + this->magnitude_ * this->rnd_distribution_(this->rnd_generator_));
    if(this->phase_ > Constants::M2PI)
    {
        this->phase_ -= Constants::M2PI;
    }

    return out;
}

//////////////////////////////////
// softSaturate implemlentation //
//////////////////////////////////
float32_t softSaturate(float32_t in, float32_t threshold)
{
    float32_t x = std::abs(in);
    float32_t y = 0.0f;
    if(x < threshold)
    {
        y = x;
    }
    else if(x > threshold)
    { 
        y = threshold + (x - threshold) / (1.0f + std::pow((x - threshold) / (1.0f - threshold), 2.0f));
    }
    else if(x > 1.0f)
    {
        y = (threshold + 1.0f) / 2.0f;
    }

    float32_t g = 2.0f / (1.0f + threshold);
    y *= g;
    
    return (in < 0.0f) ? -y : y;
}
