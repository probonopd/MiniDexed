#include "fx_phaser.h"

#include <cmath>

PhaserParameter::PhaserParameter(float32_t sampling_rate, float32_t frequency, float32_t resonance) :
    FXBase(sampling_rate),
    frequency_(frequency),
    resonance_(resonance)
{
    this->computeCoefficients();
}

PhaserParameter::~PhaserParameter()
{
}

void PhaserParameter::computeCoefficients()
{
    float32_t w0 = 2.0f * PI * this->getFrequency() / this->getSamplingRate();
    float32_t alpha = sin(w0) / (2.0f * this->resonance_);
    this->a0 = 1.0f + alpha;
    this->a1 = -2.0f * cos(w0);
    this->a2 = 1.0f - alpha;
    this->b1 = this->a1;
    this->b2 = this->a2;
}

void PhaserParameter::setFrequency(float32_t frequency)
{
    this->frequency_ = frequency;
    this->computeCoefficients();
}

float32_t PhaserParameter::getFrequency() const
{
    return this->frequency_;
}

void PhaserParameter::setResonance(float32_t resonance)
{
    this->resonance_ = constrain(resonance, 0.5f, 10.0f);
    this->computeCoefficients();
}

float32_t PhaserParameter::getResonance() const
{
    return this->resonance_;
}



// PhaserStage implementation
PhaserStage::PhaserStage(float32_t sampling_rate, PhaserParameter* params) :
    FXElement(sampling_rate),
    params_(params)
{
    memset(this->z1, 0, 2 * sizeof(float32_t));
    memset(this->z2, 0, 2 * sizeof(float32_t));
}

void PhaserStage::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = (this->params_->a0 * inL + this->params_->a1 * this->z1[0] + this->params_->a2 * this->z2[0]) / this->params_->a0;
    this->z2[0] = this->z1[0];
    this->z2[0] = inL;

    outR = (this->params_->a0 * inR + this->params_->a1 * this->z1[1] + this->params_->a2 * this->z2[1]) / this->params_->a0;
    this->z2[1] = this->z1[1];
    this->z2[1] = inR;
}



// Phaser implementation
Phaser::Phaser(float32_t sampling_rate, float32_t frequency, float32_t q) : 
    FXElement(sampling_rate),
    params_(sampling_rate, frequency, q),
    lfo_(sampling_rate, LFO::Waveform::Sine, 0.01f, 1.0f)
{
    for(unsigned i = 0; i < NUM_PHASER_STAGES; ++i)
    {
        this->stages_[i] = new PhaserStage(sampling_rate, &this->params_);
    }
}

Phaser::~Phaser()
{
    for(unsigned i = 0; i < NUM_PHASER_STAGES; ++i)
    {
        delete this->stages_[i];
    }
}

void Phaser::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Process the input sample through each stage of the phaser
    float32_t sampleL = inL;
    float32_t sampleR = inR;
    for(unsigned s = 0; s < NUM_PHASER_STAGES; ++s)
    {
        this->stages_[s]->processSample(sampleL, sampleR, sampleL, sampleR);
    }

    // Modulate the output of the phaser using the LFO 
    float32_t lfo = this->lfo_.process();
    outR = sampleR * (0.5f + 0.5f * lfo);
    outL = sampleL * (0.5f + 0.5f * lfo);
}

void Phaser::setFrequency(float32_t frequency)
{
    this->lfo_.setNormalizedFrequency(frequency);
    this->params_.setFrequency(this->lfo_.getFrequency());
}

inline float32_t Phaser::getFrequency() const
{
    return this->lfo_.getNormalizedFrequency();
}

void Phaser::setResonance(float32_t q)
{
    this->params_.setResonance(q);
}

inline float32_t Phaser::getResonance() const
{
    return this->params_.getResonance();
}
