#include "fx_svf.h"

#include <cmath>

StateVariableFilter::StateVariableFilter(float32_t sampling_rate, Type type, float32_t cutoff) :
    FXElement(sampling_rate),
    type_(type),
    gain_(-1.0f),
    cutoff_(cutoff),
    resonance_(0.0f)
{
    this->setCutoff(cutoff);
    this->setResonance(0.0f);
    this->setGainDB(0.0f);

    this->reset();
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

void StateVariableFilter::setGainDB(float32_t gainDB)
{
    gainDB = constrain(gainDB, -1.0f, 1.0f);
    if(this->gain_ != gainDB)
    {
        this->gain_ = gainDB;
        this->g_ = std::pow(10.0f, 1.2f * this->gain_);
        this->updateCoefficients();
    }
}

void StateVariableFilter::updateCoefficients()
{
    // Compute the filter coefficients based on the current parameter values
    this->w_ = 2.0f * std::tan(PI * this->cutoff_ / this->getSamplingRate());
    this->a_ = this->w_ / this->resonance_;
    this->b_ = this->w_ * this->w_;
    float32_t a_b = this->a_ + this->b_;
    this->c1_ = a_b / (1.0f + 0.5f * this->a_ + 0.25f * this->b_);
    this->c2_ = this->b_ / a_b;

    switch(this->type_)
    {

    case Type::LPF:
        this->d1_ = 0.0f;
        this->d0_ = 0.25f * this->c1_ * this->c2_;
        break;
    case Type::HPF:
        this->d1_ = 0.0f;
        this->d0_ = 1.0f - 0.5f * this->c1_ + 0.25f * this->c1_ * this->c2_;
        break;
    case Type::BPF:
        this->d1_ = 1.0f - this->c2_;
        this->d0_ = this->d1_ * this->c1_ * 0.5f;
        break;
    }

    this->reset();
}

void StateVariableFilter::reset()
{
    memset(this->z1_, 0, StereoChannels::kNumChannels * sizeof(float32_t));
    memset(this->z2_, 0, StereoChannels::kNumChannels * sizeof(float32_t));
}

void StateVariableFilter::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    const float32_t gain = this->g_;

    switch(this->type_)
    {
    case Type::LPF:
        {
            const float32_t x = inL - this->z1_[StereoChannels::Left] - this->z2_[StereoChannels::Left] + 1e-20f;
            this->z2_[StereoChannels::Left] += this->c2_ * this->z1_[StereoChannels::Left];
            outL = gain * (this->d0_ * x + this->z2_[StereoChannels::Left]);
            this->z1_[StereoChannels::Left] += this->c1_ * x;
        }
        {
            const float32_t x = inR - this->z1_[StereoChannels::Right] - this->z2_[StereoChannels::Right] + 1e-20f;
            this->z2_[StereoChannels::Right] += this->c2_ * this->z1_[StereoChannels::Right];
            outR = gain * (this->d0_ * x + this->z2_[StereoChannels::Right]);
            this->z1_[StereoChannels::Right] += this->c1_ * x;
        }
        break;

    case Type::HPF:
        {
            const float32_t x = inL - this->z1_[StereoChannels::Left] - this->z2_[StereoChannels::Left] + 1e-20f;
            outL = gain * this->d0_ * x;
            this->z2_[StereoChannels::Left] += this->c2_ * this->z1_[StereoChannels::Left];
            this->z1_[StereoChannels::Left] += this->c1_ * x;
        }
        {
            const float32_t x = inR - this->z1_[StereoChannels::Right] - this->z2_[StereoChannels::Right] + 1e-20f;
            outR = gain * this->d0_ * x;
            this->z2_[StereoChannels::Right] += this->c2_ * this->z1_[StereoChannels::Right];
            this->z1_[StereoChannels::Right] += this->c1_ * x;
        }
        break;

    case Type::BPF:
        {
            const float32_t x = inL - this->z1_[StereoChannels::Left] - this->z2_[StereoChannels::Left] + 1e-20f;
            outL = gain * (this->d0_ * x) + this->d1_ * this->z1_[StereoChannels::Left];
            this->z2_[StereoChannels::Left] += this->c2_ * this->z1_[StereoChannels::Left];
            this->z1_[StereoChannels::Left] += this->c1_ * x;
        }
        {
            const float32_t x = inR - this->z1_[StereoChannels::Right] - this->z2_[StereoChannels::Right] + 1e-20f;
            outL = gain * (this->d0_ * x) + this->d1_ * this->z1_[StereoChannels::Right];
            this->z2_[StereoChannels::Right] += this->c2_ * this->z1_[StereoChannels::Right];
            this->z1_[StereoChannels::Right] += this->c1_ * x;
        }
        break;
    }
}
