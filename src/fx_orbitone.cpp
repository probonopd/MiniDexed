#include "fx_orbitone.h"

#include <cmath>

OrbitoneParameter::OrbitoneParameter(float32_t sampling_rate, float32_t feedback) :
    FXBase(sampling_rate),
    feedback_(feedback)
{
}

OrbitoneParameter::~OrbitoneParameter()
{
}

void OrbitoneParameter::setFeedback(float32_t feedback)
{
    this->feedback_ = constrain(feedback, 0.0f, 1.0f);
}

inline float32_t OrbitoneParameter::getFeedback() const
{
    return this->feedback_;
}



// OrbitoneStage implementation
OrbitoneStage::OrbitoneStage(float32_t sampling_rate, OrbitoneParameter* params, float32_t frequency, float32_t level) :
    FXElement(sampling_rate),
    params_(params),
    lfo_(sampling_rate, LFO::Waveform::Sine, 0.0f, 20000.0f),
    level_(level)
{
    this->lfo_.setFrequency(frequency);

    this->x_[0] = this->x_[1] = 0.0f;
}

OrbitoneStage::~OrbitoneStage()
{
}

void OrbitoneStage::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Generate a sine wave using the stage's oscillator
    float32_t osc = this->level_ * this->lfo_.process();

    // Apply feedback to the stage's input
    outL = inL + inL * osc + this->params_->getFeedback() * this->x_[0];
    outR = inR + inR * osc + this->params_->getFeedback() * this->x_[1];
}



// Orbitone implementation
Orbitone::Orbitone(float32_t sampling_rate, float32_t feedback) : 
    FXElement(sampling_rate),
    params_(sampling_rate, feedback)
{
    float32_t level = 1.0f;
    for(unsigned i = 0; i < NUM_ORBITONR_STAGES; ++i)
    {
        float32_t frequency = 440.0 * pow(2.0f, (i - 1) / 12.0f);   // Sets the frequency of each stage to be a multiple of 440 Hz
        level /= 2.0f;
        this->stages_[i] = new OrbitoneStage(sampling_rate, &this->params_, frequency, level);
    }
}

Orbitone::~Orbitone()
{
    for(unsigned i = 0; i < NUM_ORBITONR_STAGES; ++i)
    {
        delete this->stages_[i];
    }
}

void Orbitone::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Process the input sample through each stage of the phaser
    float32_t sampleL = inL;
    float32_t sampleR = inR;
    for(unsigned s = 0; s < NUM_ORBITONR_STAGES; ++s)
    {
        this->stages_[s]->processSample(sampleL, sampleR, sampleL, sampleR);
    }

    // Modulate the output of the phaser using the LFO 
    outL = sampleL;
    outR = sampleR;
}

void Orbitone::setFeedback(float32_t feedback)
{
    this->params_.setFeedback(feedback);
}

float32_t Orbitone::getFeedback() const
{
    return this->params_.getFeedback();
}
