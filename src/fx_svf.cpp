#include "fx_svf.h"

#include <cmath>

StateVariableFilter::StateVariableFilter(float32_t sampling_rate, Type type, float32_t cutoff) :
    FXElement(sampling_rate),
    type_(type),
    cutoff_(0.0f)
{
    memset(this->z1_, 0, 2 * sizeof(float32_t));
    memset(this->z2_, 0, 2 * sizeof(float32_t));

    this->setCutoff(cutoff);
    this->setResonance(0.0f);
}

StateVariableFilter::~StateVariableFilter()
{
}

void StateVariableFilter::setFilterType(Type type)
{
    if(this->type_ != type)
    {
        this->type_ = type;
        this->updateCoefficients();
    }
}

void StateVariableFilter::setCutoff(float32_t cutoff)
{
    cutoff = constrain(cutoff, 1.0f, this->getSamplingRate() / 2.0f);
    if(this->cutoff_ != cutoff)
    {
        this->cutoff_ = cutoff;
        this->updateCoefficients();
    }
}

void StateVariableFilter::setResonance(float32_t resonance)
{
    resonance = constrain(resonance, 0.005f, 1.0f);
    if(this->resonance_ != resonance)
    {
        this->resonance_ = resonance;
        this->updateCoefficients();
    }
}

void StateVariableFilter::setPeakGainDB(float32_t gain)
{
    if(this->peak_gain_ != gain)
    {
        this->peak_gain_ = gain;
        this->updateCoefficients();
    }
}

void StateVariableFilter::updateCoefficients()
{
    // Compute the filter coefficients based on the current parameter values
    float32_t w0 = PI * this->cutoff_ / this->getSamplingRate();
    float32_t V = pow(10, fabs(this->peak_gain_) / 20.0f);
    float32_t K = std::tan(w0);
    float32_t K2 = K * K;
    float32_t norm;

    switch(this->type_)
    {
    case Type::LPF:
        norm = 1.0f / (1.0f + K / this->resonance_ + K2);
        this->a0_ = K2 * norm;
        this->a1_ = 2.0f * this->a0_;
        this->a2_ = this->a0_;
        this->b1_ = 2.0f * (K2 - 1.0f) * norm;
        this->b2_ = (1.0f - K / this->resonance_ + K2) * norm;
        break;
    case Type::HPF:
        norm = 1.0f / (1.0f + K / this->resonance_ + K2);
        this->a0_ = norm;
        this->a1_ = -2.0f * this->a0_;
        this->a2_ = this->a0_;
        this->b1_ = 2.0f * (K2 - 1.0f) * norm;
        this->b2_ = (1.0f - K / this->resonance_ + K2) * norm;
        break;
    case Type::BPF:
        norm = 1.0f / (1.0f + K / this->resonance_ + K2);
        this->a0_ = K / this->resonance_ * norm;
        this->a1_ = 0.0f;
        this->a2_ = -this->a0_;
        this->b1_ = 2.0f * (K2 - 1.0f) * norm;
        this->b2_ = (1.0f - K / this->resonance_ + K2) * norm;
        break;
    case Type::NOTCH:
        norm = 1.0f / (1.0f + K / this->resonance_ + K2);
        this->a0_ = (1.0f + K2) * norm;
        this->a1_ = 2.0f * (K2 - 1.0f) * norm;
        this->a2_ = this->a0_;
        this->b1_ = 2.0f * (K2 - 1.0f) * norm;
        this->b2_ = (1.0f - K / this->resonance_ + K2) * norm;
        break;
    case Type::PEQ:
        if(this->peak_gain_ >= 0) 
        {
            // boost
            norm = 1.0f / (1.0f + 1.0f / this->resonance_ * K + K2);
            this->a0_ = (1.0f + V / this->resonance_ * K + K2) * norm;
            this->a1_ = 2.0f * (K2 - 1) * norm;
            this->a2_ = (1.0f - V / this->resonance_ * K + K2) * norm;
            this->b1_ = this->a1_;
            this->b2_ = (1.0f - 1.0f / this->resonance_ * K + K2) * norm;
        }
        else
        {
            // cut
            norm = 1.0f / (1 + V / this->resonance_ * K + K2);
            this->a0_ = (1.0f + 1.0f / this->resonance_ * K + K2) * norm;
            this->a1_ = 2.0f * (K2 - 1) * norm;
            this->a2_ = (1.0f - 1.0f / this->resonance_ * K + K2) * norm;
            this->b1_ = this->a1_;
            this->b2_ = (1.0f - V / this->resonance_ * K + K2) * norm;
        }
        break;
    case Type::LSH:
        if(this->peak_gain_ >= 0)
        {    
            // boost
            norm = 1 / (1 + std::sqrt(2) * K + K2);
            this->a0_ = (1.0f + std::sqrt(2.0f * V) * K + V * K2) * norm;
            this->a1_ = 2.0f * (V * K2 - 1.0f) * norm;
            this->a2_ = (1.0f - std::sqrt(2.0f * V) * K + V * K2) * norm;
            this->b1_ = 2.0f * (K2 - 1.0f) * norm;
            this->b2_ = (1.0f - std::sqrt(2.0f) * K + K2) * norm;
        }
        else
        {   
            // cutK * K
            norm = 1.0f / (1.0f + std::sqrt(2.0f * V) * K + V * K2);
            this->a0_ = (1.0f + std::sqrt(2.0f) * K + K2) * norm;
            this->a1_ = 2.0f * (K2 - 1.0f) * norm;
            this->a2_ = (1.0f - std::sqrt(2.0f) * K + K2) * norm;
            this->b1_ = 2.0f * (V * K2 - 1.0f) * norm;
            this->b2_ = (1.0f - std::sqrt(2.0f * V) * K + V * K2) * norm;
        }
        break;
    case Type::HSH:
        if(this->peak_gain_ >= 0)
        {
            // boost
            norm = 1.0f / (1.0f + std::sqrt(2.0f) * K + K2);
            this->a0_ = (V + std::sqrt(2.0f * V) * K + K2) * norm;
            this->a1_ = 2.0f * (K2 - V) * norm;
            this->a2_ = (V - std::sqrt(2.0f * V) * K + K2) * norm;
            this->b1_ = 2.0f * (K2 - 1.0f) * norm;
            this->b2_ = (1.0f - std::sqrt(2.0f) * K + K2) * norm;
        }
        else
        {
            // cut
            norm = 1.0f / (V + std::sqrt(2.0f * V) * K + K2);
            this->a0_ = (1.0f + std::sqrt(2.0f) * K + K2) * norm;
            this->a1_ = 2.0f * (K2 - 1.0f) * norm;
            this->a2_ = (1.0f - std::sqrt(2.0f) * K + K2) * norm;
            this->b1_ = 2.0f * (K2 - V) * norm;
            this->b2_ = (V - std::sqrt(2.0f * V) * K + K2) * norm;
        }
        break;        
    }
}

void StateVariableFilter::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    const float32_t gain = 10.0f;

    outL = (inL * this->a0_ + this->z1_[0]) * gain;
    this->z1_[0] = inL * this->a1_ + this->z2_[0] - this->b1_ * outL;
    this->z2_[0] = inL * this->a2_ - this->b2_ * outL;

    outR = (inR * this->a0_ + this->z1_[1]) * gain;
    this->z1_[0] = inR * this->a1_ + this->z2_[1] - this->b1_ * outR;
    this->z2_[0] = inR * this->a2_ - this->b2_ * outR;
}
