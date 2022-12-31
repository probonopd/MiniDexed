#include "fx_flanger.h"

#include <cmath>

#define MAX_FLANGER_DELAY 20.0f

Flanger::Flanger(float32_t sampling_rate, float32_t delay_time, float32_t frequency, float32_t depth, float32_t feedback) :
    FXElement(sampling_rate),
    MaxDelayLineSize(static_cast<unsigned>(2.0f * MAX_FLANGER_DELAY * sampling_rate / 1000.0f)),
    delay_line_index_(0),
    lfo_(sampling_rate, LFO::Waveform::Sine, 0.1f, 10.0f)
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
    // Calculate the delay time based on the depth and rate parameters
    float32_t delay = this->getDelayTime() + this->getDepth() * this->lfo_.process();

    // Convert the delay time to samples
    unsigned delay_samples = static_cast<unsigned>(delay * this->getSamplingRate() / 1000.0f);

    // mix the input audio with the delayed audio and the feedback signal
    outL = inL + this->delay_lineL_[(this->delay_line_index_ + this->delay_line_size_ - delay_samples) % this->delay_line_size_] * (1.0 - this->getFeedback());
    outR = inR + this->delay_lineR_[(this->delay_line_index_ + this->delay_line_size_ - delay_samples) % this->delay_line_size_] * (1.0 - this->getFeedback());

    // Update the delay buffer with the mixed audio and the feedback signal
    this->delay_lineL_[this->delay_line_index_] = inL + outL * this->getFeedback();
    this->delay_lineR_[this->delay_line_index_] = inR + outR * this->getFeedback();

    this->delay_line_index_ = (this->delay_line_index_ + 1) % this->delay_line_size_;
}

void Flanger::setDelayTime(float32_t delayMS)
{
    this->delay_time_ms_ = constrain(delayMS, 1.0f, MAX_FLANGER_DELAY);
    this->adjustDelayCofficients();
}

float32_t Flanger::getDelayTime() const
{
    return this->delay_time_ms_;
}

void Flanger::setFrequency(float32_t frequency)
{
    this->lfo_.setNormalizedFrequency(frequency);
}

float32_t Flanger::getFrequency() const
{
    return this->lfo_.getNormalizedFrequency();
}

void Flanger::setDepth(float32_t depth)
{
    this->depth_ = constrain(depth, 0.0f, MAX_FLANGER_DELAY);
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
