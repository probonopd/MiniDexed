#include "fx_phaser.h"

#include <cmath>

PhaserParameter::PhaserParameter(float32_t sampling_rate, float32_t frequency, float32_t q) :
    FXBase(sampling_rate),
    frequency_(frequency),
    q_(q)
{
    this->computeCoefficients();
}

PhaserParameter::~PhaserParameter()
{
}

void PhaserParameter::computeCoefficients()
{
    float32_t w0 = 2.0f * PI * this->getFrequency() / this->getSamplingRate();
    float32_t alpha = sin(w0) / (2.0f * this->q_);
    this->a0 = 1.0f + alpha;
    this->a1 = -2.0f * cos(w0);
    this->a2 = 1.0f - alpha;
    this->b1 = this->a1;
    this->b2 = this->a2;
}

void PhaserParameter::setFrequency(float32_t frequency)
{
    this->frequency_ = constrain(frequency, 0.1, 10.0);
    this->computeCoefficients();
}

float32_t PhaserParameter::getFrequency() const
{
    return this->frequency_;
}

void PhaserParameter::setQ(float32_t q)
{
    this->q_ = constrain(q, 0.5f, 10.0f);
    this->computeCoefficients();
}

float32_t PhaserParameter::getQ() const
{
    return this->q_;
}



// PhaserStage implementation
PhaserStage::PhaserStage(float32_t sampling_rate, PhaserParameter* params) :
    FXBase(sampling_rate),
    params_(params)
{
    memset(this->z1, 0, 2 * sizeof(float32_t));
    memset(this->z2, 0, 2 * sizeof(float32_t));
}

void PhaserStage::process(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
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
    FX(sampling_rate),
    params_(sampling_rate, frequency, q),
    phase_(0.0f),
    phase_increment_(0.0f)
{
    this->phase_increment_ = 2.0f * PI * frequency / this->getSamplingRate();
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

void Phaser::process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples)
{
    float sampleL;
    float sampleR;
    for(unsigned i = 0; i < nSamples; ++i)
    {
        // Process the input sample through each stage of the phaser
        sampleL = *left_input;
        sampleR = *right_input;
        for(unsigned s = 0; s < NUM_PHASER_STAGES; ++s)
        {
            this->stages_[s]->process(sampleL, sampleR, sampleL, sampleR);
        }

        // Modulate the output of the phaser using the LFO 
        *left_output = sampleL * (0.5f + 0.5f * cos(this->phase_));
        *right_output = sampleR * (0.5f + 0.5f * cos(this->phase_));;

        // Update the phase of the LFO
        this->phase_ += this->phase_increment_;
        if(this->phase_ > 2.0f * PI) {
            this->phase_ -= 2.0 * PI;
        }

        // Move to next input sample
        ++left_input;
        ++right_input;

        // Move to next output sample
        ++left_output;
        ++right_output;
    }
}

void Phaser::setFrequency(float32_t frequency)
{
    this->params_.setFrequency(frequency);
    this->phase_increment_ = 2.0f * PI * frequency / this->getSamplingRate();
}

float32_t Phaser::getFrequency() const
{
    return this->params_.getFrequency();
}

void Phaser::setQ(float32_t q)
{
    this->params_.setQ(q);
}

float32_t Phaser::getQ() const
{
    return this->params_.getQ();
}
