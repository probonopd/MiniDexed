#include "fx_phaser.h"

#include <algorithm>
#include <cmath>

Phaser::AllpassDelay::AllpassDelay() :
    FXElement(0.0f),
    a1_(0.0f)
{
    this->reset();
}

Phaser::AllpassDelay::~AllpassDelay()
{
}

void Phaser::AllpassDelay::reset()
{
    memset(this->z_, 0, 2 * sizeof(float32_t));
}

void Phaser::AllpassDelay::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = inL * -this->a1_ + this->z_[0];
    this->z_[0] = outL * this->a1_ + inL;

    outR = inR * -this->a1_ + this->z_[1];
    this->z_[1] = outR * this->a1_ + inR;
}

void Phaser::AllpassDelay::setDelay(float32_t delay)
{
    this->a1_ = (1.0f - delay) / (1.0f + delay);
}


Phaser::Phaser(float32_t sampling_rate, float32_t rate, float32_t depth, float32_t feedback, unsigned nb_stages) : 
    FXElement(sampling_rate),
    lfo_(sampling_rate, 0.0f, 2.5f),
    depth_(0.0f),
    feedback_(0.0f),
    dmin_(0.0f),
    dmax_(0.0f)
{
    this->setRate(rate);
    this->setDepth(depth);
    this->setFeedback(feedback);
    this->setNbStages(nb_stages);
    this->setFrequencyRange(440.0f, 1600.0f);

    this->reset();
}

Phaser::~Phaser()
{
}

void Phaser::reset()
{
    memset(this->z_, 0, 2 * sizeof(float32_t));

    for(unsigned i = 0; i < MAX_NB_PHASES; ++i)
    {
        this->stages_[i].reset();
    }
    this->lfo_.reset();
}

void Phaser::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    float32_t d = this->dmin_ + (this->dmax_ - this->dmin_) * ((1.0f + this->lfo_.process()) / 2.0f);

    float32_t sampleL = inL + this->feedback_ * this->z_[0];
    float32_t sampleR = inR + this->feedback_ * this->z_[1];
    for(unsigned i = 0; i < this->nb_stages_; ++i)
    {
        this->stages_[i].setDelay(d);

        this->stages_[i].processSample(sampleL, sampleR, sampleL, sampleR);
    }
    this->z_[0] = sampleL;
    this->z_[1] = sampleR;

    outL = inL + this->z_[StereoChannels::Left ] * this->depth_;
    outR = inR + this->z_[StereoChannels::Right] * this->depth_;
}

void Phaser::setFrequencyRange(float32_t min_frequency, float32_t max_frequency)
{
    this->dmin_ = 2.0f * std::min(min_frequency, max_frequency) / this->getSamplingRate();
    this->dmax_ = 2.0f * std::max(min_frequency, max_frequency) / this->getSamplingRate();
}

void Phaser::setRate(float32_t rate)
{
    rate = constrain(rate, 0.0f, 1.0f);
    this->lfo_.setNormalizedFrequency(rate);
}

inline float32_t Phaser::getRate() const
{
    return this->lfo_.getNormalizedFrequency();
}

void Phaser::setDepth(float32_t depth)
{
    depth = constrain(depth, 0.0f, 1.0f);
    this->depth_ = depth;
}

inline float32_t Phaser::getDepth() const
{
    return this->depth_;
}

void Phaser::setFeedback(float32_t feedback)
{
    feedback = constrain(feedback, 0.0f, 0.97f);
    this->feedback_ = feedback;
}

inline float32_t Phaser::getFeedback() const
{
    return this->feedback_;
}

void Phaser::setNbStages(unsigned nb_stages)
{
    if(nb_stages < 2)
    {
        nb_stages = 2;
    }
    else if(nb_stages > MAX_NB_PHASES)
    {
        nb_stages = MAX_NB_PHASES;
    }
    this->nb_stages_ = nb_stages;
}

unsigned Phaser::getNbStages() const
{
    return this->nb_stages_;
}
