#include "fx_flanger.h"

#include <cmath>

#define MAX_FLANGER_DELAY 20.0f

Flanger::Flanger(float32_t sampling_rate, float32_t delay_time, float32_t frequency, float32_t depth, float32_t feedback) :
    FXElement(sampling_rate),
    MaxDelayLineSize(static_cast<unsigned>(MAX_FLANGER_DELAY * sampling_rate / 1000.0f)),
    lfo_phase_(0.0f)
{
    this->delay_lineL_ = new float32_t[this->MaxDelayLineSize];
    this->delay_lineR_ = new float32_t[this->MaxDelayLineSize];

    this->setDelayTime(delay_time);
    this->setFrequency(frequency);
    this->setDepth(depth);
    this->setFeedback(feedback);
}

Flanger::~Flanger()
{
    delete[] this->delay_lineL_;
    delete[] this->delay_lineR_;
}

void Flanger::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Modulate the delay time using the LFO
    float32_t delay = this->delay_time_ms_ + this->depth_ * sin(this->lfo_phase_);

    // Calculate the delay line index and interpolate between samples
    int index = (i - (int) (this->getSamplingRate() * delay / 1000.0f)) % this->delay_line_size_;
    float32_t frac = (this->getSamplingRate() * delay / 1000.0f) - (int) (this->getSamplingRate() * delay / 1000.0f);
    float32_t x1 = this->delay_lineL_[index];
    float32_t x2 = this->delay_lineL_[(index + 1) % this->delay_line_size_];
    float32_t sample = x1 + frac * (x2 - x1);

    // Process the input sample through the flanger
    outL = inL + sample * this->feedback_;
    outR = inR + sample * this->feedback_;

    // Update the delay line
    this->delay_lineL_[i % this->delay_line_size_] = outL;
    this->delay_lineR_[i % this->delay_line_size_] = outR;

    // Update the phase of the LFO
    this->lfo_phase_ += this->lfo_phase_increment_;
    if(this->lfo_phase_ > 2.0 * PI) {
        this->lfo_phase_ -= 2.0 * PI;
    }
}

void Flanger::setDelayTime(float32_t delayMS)
{
    this->delay_time_ms_ = constrain(delayMS, 1.0f, 10.0f);
    this->adjustDelayCofficients();
}

float32_t Flanger::getDelayTime() const
{
    return this->delay_time_ms_;
}

void Flanger::setFrequency(float32_t frequency)
{
    frequency = constrain(frequency, 0.1f, 1.0f);
    this->frequency_ = frequency;
    this->lfo_phase_increment_ = 2.0f * PI * frequency / this->getSamplingRate();
}

float32_t Flanger::getFrequency() const
{
    return this->frequency_;
}

void Flanger::setDepth(float32_t depth)
{
    this->depth_ = constrain(depth, 0.0f, 10.0f);
    this->adjustDelayCofficients();
}

float32_t Flanger::getDepth() const
{
    return this->depth_;
}

void Flanger::setFeedback(float32_t feedback)
{
    this->feedback_ = constrain(feedback, 0.0f, 1.0f);
}

float32_t Flanger::getFeedback() const
{
    return this->feedback_;
}

void Flanger::adjustDelayCofficients()
{
    this->delay_line_size_ = static_cast<unsigned>(this->getSamplingRate() * (this->getDelayTime() + this->getDepth()) / 1000.0f);
}
