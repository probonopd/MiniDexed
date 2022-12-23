#include "fx_phaser.h"

#include <cmath>

PhaserStage::PhaserStage(float32_t sampling_rate, float32_t frequency, float32_t q) :
    FXBase(sampling_rate),
    frequency_(frequency),
    q_(q)
{
    memset(this->z1, 0, 2 * sizeof(float32_t));
    memset(this->z2, 0, 2 * sizeof(float32_t));
    this->computeCoefficients();
}

void PhaserStage::computeCoefficients()
{
    float32_t w0 = 2.0f * PI * this->getFrequency() / this->getSamplingRate();
    float32_t alpha = sin(w0) / (2.0f * this->q_);
    this->a0 = 1.0f + alpha;
    this->a1 = -2.0f * cos(w0);
    this->a2 = 1.0f - alpha;
    this->b1 = this->a1;
    this->b2 = this->a2;
}

void PhaserStage::process(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = (this->a0 * inL + this->a1 * this->z1[0] + this->a2 * this->z2[0]) / this->a0;
    this->z2[0] = this->z1[0];
    this->z2[0] = inL;

    outR = (this->a0 * inR + this->a1 * this->z1[1] + this->a2 * this->z2[1]) / this->a0;
    this->z2[1] = this->z1[1];
    this->z2[1] = inR;
}

void PhaserStage::setFrequency(float32_t frequency)
{
    this->frequency_ = constrain(frequency, 0.0, 10.0);
    this->computeCoefficients();
}

float32_t PhaserStage::getFrequency() const
{
    return this->frequency_;
}

void PhaserStage::setQ(float32_t q)
{
    this->q_ = constrain(q, 0.1f, 1.0f);
    this->computeCoefficients();
}

float32_t PhaserStage::getQ() const
{
    return this->q_;
}

