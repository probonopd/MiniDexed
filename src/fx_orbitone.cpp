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

float32_t OrbitoneParameter::getFeedback() const
{
    return this->feedback_;
}



// OrbitoneStage implementation
OrbitoneStage::OrbitoneStage(float32_t sampling_rate, OrbitoneParameter* params, float32_t frequency) :
    FXElement(sampling_rate),
    params_(params),
    frequency_(frequency),
    phase_(0.0f)
{
    this->phase_increment_ = 2.0f * PI * frequency / this->getSamplingRate();
}

void OrbitoneStage::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Generate a sine wave using the stage's oscillator
    float32_t osc = sin(this->phase_);

    // Update the phase of the oscillator
    this->phase_ += this->phase_increment_;
    if(this->phase_ > 2.0f * PI) {
        this->phase_ -= 2.0f * PI;
    }

    // Apply feedback to the stage's input
    outL = inL + osc * this->params_->getFeedback();
    outR = inR + osc * this->params_->getFeedback();
}



// Orbitone implementation
Orbitone::Orbitone(float32_t sampling_rate, float32_t feedback) : 
    FXElement(sampling_rate),
    params_(sampling_rate, feedback)
{
    for(unsigned i = 0; i < NUM_ORBITONR_STAGES; ++i)
    {
        float32_t frequency = 440.0 * pow(2.0f, (i - 1) / 12.0f);   // Sets the frequency of each stage to be a multiple of 440 Hz
        this->stages_[i] = new OrbitoneStage(sampling_rate, &this->params_, frequency);
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
