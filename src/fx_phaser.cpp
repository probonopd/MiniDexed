#include "fx_phaser.h"

Phaser::AllpassDelay::AllpassDelay() :
    FXElement(0.0f)
{
    this->reset();
}

Phaser::AllpassDelay::~AllpassDelay()
{
}

void Phaser::AllpassDelay::reset()
{
    memset(this->a1_, 0, StereoChannels::kNumChannels * sizeof(float32_t));
    memset(this->z_, 0, StereoChannels::kNumChannels * sizeof(float32_t));
}

void Phaser::AllpassDelay::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = inL * -this->a1_[StereoChannels::Left ] + this->z_[StereoChannels::Left ];
    this->z_[StereoChannels::Left ] = outL * this->a1_[StereoChannels::Left ] + inL;

    outR = inR * -this->a1_[StereoChannels::Right] + this->z_[StereoChannels::Right];
    this->z_[StereoChannels::Right] = outR * this->a1_[StereoChannels::Right] + inR;
}

void Phaser::AllpassDelay::setDelay(float32_t delayL, float32_t delayR)
{
    this->a1_[StereoChannels::Left ] = (1.0f - delayL) / (1.0f + delayL);
    this->a1_[StereoChannels::Right] = (1.0f - delayR) / (1.0f + delayR);
}


Phaser::Phaser(float32_t sampling_rate, float32_t rate, float32_t depth, float32_t feedback, unsigned nb_stages) : 
    FXElement(sampling_rate),
    depth_(0.0f),
    gain_(1.0f),
    feedback_(0.0f),
    dmin_(0.0f),
    dmax_(0.0f)
{
    this->lfo_[StereoChannels::Left ] = new LFO(sampling_rate, 0.0f, 2.5f, 0.0f, false);
    this->lfo_[StereoChannels::Right] = new LFO(sampling_rate, 0.0f, 2.5f, Constants::MPI_2, false);

    this->setRate(rate);
    this->setDepth(depth);
    this->setFeedback(feedback);
    this->setNbStages(nb_stages);
    this->setFrequencyRange(440.0f, 1600.0f);

    this->reset();
}

Phaser::~Phaser()
{
    delete this->lfo_[StereoChannels::Left ];
    delete this->lfo_[StereoChannels::Right];
}

void Phaser::reset()
{
    memset(this->z_, 0, StereoChannels::kNumChannels * sizeof(float32_t));

    for(unsigned i = 0; i < MAX_NB_PHASES; ++i)
    {
        this->stages_[i].reset();
    }
    this->lfo_[StereoChannels::Left ]->reset();
    this->lfo_[StereoChannels::Right]->reset();
}

void Phaser::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    float32_t dL = this->dmin_ + (this->dmax_ - this->dmin_) * ((1.0f + this->lfo_[StereoChannels::Left ]->process()) / 2.0f);
    float32_t dR = this->dmin_ + (this->dmax_ - this->dmin_) * ((1.0f + this->lfo_[StereoChannels::Right]->process()) / 2.0f);

    float32_t sampleL = inL + this->feedback_ * this->z_[StereoChannels::Left ];
    float32_t sampleR = inR + this->feedback_ * this->z_[StereoChannels::Right];
    for(unsigned i = 0; i < this->nb_stages_; ++i)
    {
        this->stages_[i].setDelay(dL, dR);
        this->stages_[i].processSample(sampleL, sampleR, sampleL, sampleR);
    }
    this->z_[StereoChannels::Left ] = sampleL;
    this->z_[StereoChannels::Right] = sampleR;

    outL = inL + this->z_[StereoChannels::Left ] * this->depth_;
    outR = inR + this->z_[StereoChannels::Right] * this->depth_;

    outL *= this->gain_;
    outR *= this->gain_;
}

void Phaser::setFrequencyRange(float32_t min_frequency, float32_t max_frequency)
{
    this->dmin_ = 2.0f * std::min(min_frequency, max_frequency) / this->getSamplingRate();
    this->dmax_ = 2.0f * std::max(min_frequency, max_frequency) / this->getSamplingRate();
}

void Phaser::setRate(float32_t rate)
{
    rate = constrain(rate, 0.0f, 1.0f);
    this->lfo_[StereoChannels::Left ]->setNormalizedFrequency(rate);
    this->lfo_[StereoChannels::Right]->setNormalizedFrequency(rate);
}

float32_t Phaser::getRate() const
{
    return this->lfo_[StereoChannels::Left]->getNormalizedFrequency();
}

void Phaser::setDepth(float32_t depth)
{
    depth = constrain(depth, 0.0f, 1.0f);
    this->depth_ = depth;
    this->gain_ = this->OutputLevelCorrector / (1.0f + depth);
}

float32_t Phaser::getDepth() const
{
    return this->depth_;
}

void Phaser::setFeedback(float32_t feedback)
{
    feedback = constrain(feedback, 0.0f, 0.97f);
    this->feedback_ = feedback;
}

float32_t Phaser::getFeedback() const
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
