#include "fx_chorus.h"

#include <cmath>

#define LFO1_MAX_FREQ 0.25f
#define LFO2_MAX_FREQ 0.35f

Chorus::Chorus(float32_t sampling_rate) :
    FXElement(sampling_rate, 1.18f),
    engine_(sampling_rate, 0.0f),
    rate_(0.0f),
    depth_(0.0f),
    fullscale_depth_(0.0f),
    feedback_(0.0f)
{
    this->lfo_[LFOIndex::Sin1] = new LFO(sampling_rate, 0.0f, LFO1_MAX_FREQ);
    this->lfo_[LFOIndex::Cos1] = new LFO(sampling_rate, 0.0f, LFO1_MAX_FREQ, Constants::MPI_2);
    this->lfo_[LFOIndex::Sin2] = new LFO(sampling_rate, 0.0f, LFO2_MAX_FREQ);
    this->lfo_[LFOIndex::Cos2] = new LFO(sampling_rate, 0.0f, LFO2_MAX_FREQ, Constants::MPI_2);

    this->setRate(0.1f);
    this->setDepth(0.15f);
}

Chorus::~Chorus()
{
    for(unsigned i = 0; i < LFOIndex::kLFOCount; ++i)
    {
        delete this->lfo_[i];
    }
}

void Chorus::reset()
{
    this->engine_.reset();
    for(unsigned i = 0; i < LFOIndex::kLFOCount; ++i)
    {
        this->lfo_[i]->reset();
    }
}

void Chorus::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    typedef Engine::Reserve<2047> Memory;
    static Engine::DelayLine<Memory, 0> line;
    Engine::Context c;

    this->engine_.start(&c);
    
    // Update LFO.
    float32_t sin_1 = this->lfo_[LFOIndex::Sin1]->process();
    float32_t cos_1 = this->lfo_[LFOIndex::Cos1]->process();
    float32_t sin_2 = this->lfo_[LFOIndex::Sin2]->process();
    float32_t cos_2 = this->lfo_[LFOIndex::Cos2]->process();

    float32_t wet;

    // Sum L & R channel to send to chorus line.
    c.read(inL + inR, 0.5f);
    c.writeAndLoad(line, 0.0f);

    c.interpolate(line, sin_1 * this->fullscale_depth_ + 1200, 0.5f);
    c.interpolate(line, sin_2 * this->fullscale_depth_ + 800, 0.5f);
    c.writeAndLoad(wet, 0.0f);
    outL = wet * this->OutputLevelCorrector;
    
    c.interpolate(line, cos_1 * this->fullscale_depth_ + 800, 0.5f);
    c.interpolate(line, cos_2 * this->fullscale_depth_ + 1200, 0.5f);
    c.writeAndLoad(wet, 0.0f);
    outR = wet * this->OutputLevelCorrector;
}

void Chorus::setRate(float32_t rate)
{
    this->rate_ = constrain(rate, 0.0f, 1.0f);
    for(unsigned i = 0; i < LFOIndex::kLFOCount; ++i)
    {
        this->lfo_[i]->setNormalizedFrequency(this->rate_);
    }
}

float32_t Chorus::getRate() const
{
    return this->depth_;
}

void Chorus::setDepth(float32_t depth)
{
    depth = constrain(depth, 0.0f, 1.0f);
    if(this->depth_ != depth)
    {
        this->depth_ = depth;
        this->fullscale_depth_ = this->depth_ * CHORUS_FULLSCALE_DEPTH_RATIO;
    }
}

float32_t Chorus::getDepth() const
{
    return this->depth_;
}
