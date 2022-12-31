#include "fx_chorus.h"

#include <cmath>

#define CHORUS_BUFFER_SIZE 8192

Chorus::Chorus(float32_t sampling_rate, unsigned voices, float32_t rate, float32_t depth, float32_t feedback) :
    FXElement(sampling_rate),
    NumVoices(voices),
    sample_position_ratio_(sampling_rate / 1000.0f),
    lfo_(sampling_rate, LFO::Waveform::Sine, 0.01f, 1.0f)
{
    this->delay_buffersL_ = new float32_t*[this->NumVoices];
    this->delay_buffersR_ = new float32_t*[this->NumVoices];

    for(unsigned i = 0; i < this->NumVoices; ++i)
    {
        this->delay_buffersL_[i] = new float32_t[CHORUS_BUFFER_SIZE];
        this->delay_buffersR_[i] = new float32_t[CHORUS_BUFFER_SIZE];
    }

    this->delay_buffer_indices_ = new unsigned[this->NumVoices];
    memset(this->delay_buffer_indices_, 0, this->NumVoices * sizeof(float32_t));

    this->setRate(rate);
    this->setDepth(depth);
    this->setFeedback(feedback);
}

Chorus::~Chorus()
{
    for(unsigned i = 0; i < this->NumVoices; ++i)
    {
        delete[] this->delay_buffersL_[i];
        delete[] this->delay_buffersR_[i];
    }

    delete[] this->delay_buffersL_;
    delete[] this->delay_buffersR_;
    delete[] this->delay_buffer_indices_;
}

void Chorus::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    float32_t sumL = 0.0f;
    float32_t sumR = 0.0f;
    for(unsigned i = 0; i < this->NumVoices; ++i) {
        // Calculate the delay time based on the depth and rate parameters
        float32_t delay = this->getDepth() * this->lfo_.process();

        // Convert the delay time to samples
        unsigned delay_samples = static_cast<unsigned>(delay * this->sample_position_ratio_);

        // Mix the input audio with the delayed audio
        sumL += inL + this->delay_buffersL_[i][(CHORUS_BUFFER_SIZE + this->delay_buffer_indices_[i] - delay_samples) % CHORUS_BUFFER_SIZE];
        sumR += inR + this->delay_buffersR_[i][(CHORUS_BUFFER_SIZE + this->delay_buffer_indices_[i] - delay_samples) % CHORUS_BUFFER_SIZE];

        // Update the delay buffer for this voice
        this->delay_buffersL_[i][this->delay_buffer_indices_[i]] = inL + sumL * this->getFeedback() / static_cast<float32_t>(i + 1);
        this->delay_buffersR_[i][this->delay_buffer_indices_[i]] = inR + sumR * this->getFeedback() / static_cast<float32_t>(i + 1);
        this->delay_buffer_indices_[i] = (delay_buffer_indices_[i] + 1) % CHORUS_BUFFER_SIZE;
    }

    // Average the mixed audio from all voices to create the output
    outL = sumL / static_cast<float32_t>(this->NumVoices);
    outR = sumR / static_cast<float32_t>(this->NumVoices);
}

void Chorus::setDepth(float32_t depth)
{
    this->depth_ = constrain(depth, 0.0f, 10.0f);
}

float32_t Chorus::getDepth() const
{
    return this->depth_;
}

void Chorus::setRate(float32_t rate)
{
    this->lfo_.setNormalizedFrequency(rate);
}

float32_t Chorus::getRate() const
{
    return this->lfo_.getNormalizedFrequency();
}

void Chorus::setFeedback(float32_t feedback)
{
    this->feedback_ = constrain(feedback, 0.0f, 1.0f);
}

float32_t Chorus::getFeedback() const
{
    return this->feedback_;
}

