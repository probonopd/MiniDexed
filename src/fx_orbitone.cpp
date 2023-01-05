#include "fx_orbitone.h"

#include <cmath>

#define LFO_SLOW_MAX_FREQUENCY 1.0f
#define LFO_FAST_MAX_FREQUENCY 8.8f

#define FULLSCALE_DEPTH_RATIO 256.0f

Orbitone::Orbitone(float32_t sampling_rate, float32_t rate, float32_t depth) : 
    FXElement(sampling_rate),
    engine_(sampling_rate, 0.0f, 0.0f),
    depth_(0.0f),
    fullscale_depth_(0.0f)
{
    this->lfo_[LFOIndex::Slow0  ] = new LFO(sampling_rate, LFO::Waveform::Sine, 0.0f, LFO_SLOW_MAX_FREQUENCY, 0.0f);
    this->lfo_[LFOIndex::Slow120] = new LFO(sampling_rate, LFO::Waveform::Sine, 0.0f, LFO_SLOW_MAX_FREQUENCY, 2.0f * PI / 3.0);
    this->lfo_[LFOIndex::Slow240] = new LFO(sampling_rate, LFO::Waveform::Sine, 0.0f, LFO_SLOW_MAX_FREQUENCY, 4.0f * PI / 3.0);

    this->lfo_[LFOIndex::Fast0  ] = new LFO(sampling_rate, LFO::Waveform::Sine, 0.0f, LFO_FAST_MAX_FREQUENCY, 0.0f);
    this->lfo_[LFOIndex::Fast120] = new LFO(sampling_rate, LFO::Waveform::Sine, 0.0f, LFO_FAST_MAX_FREQUENCY, 2.0f * PI / 3.0);
    this->lfo_[LFOIndex::Fast240] = new LFO(sampling_rate, LFO::Waveform::Sine, 0.0f, LFO_FAST_MAX_FREQUENCY, 4.0f * PI / 3.0);

    for(unsigned i = 0; i < 6; ++i)
    {
        this->lfo_[i]->setNormalizedFrequency(rate);
    }

    this->setDepth(depth);
}

Orbitone::~Orbitone()
{
    for(unsigned i = 0; i < 6; ++i)
    {
        delete this->lfo_[i];
    }
}

void Orbitone::reset()
{
    this->engine_.reset();
    for(unsigned i = 0; i < 6; ++i)
    {
        this->lfo_[i]->reset();
    }
}

void Orbitone::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    typedef Engine::Reserve<2047, Engine::Reserve<2047> > Memory;
    Engine::DelayLine<Memory, 0> line_l;
    Engine::DelayLine<Memory, 1> line_r;
    Engine::Context c;

    this->engine_.start(&c);
    
    float32_t slow_0   = this->lfo_[LFOIndex::Slow0  ]->process();
    float32_t slow_120 = this->lfo_[LFOIndex::Slow120]->process();
    float32_t slow_240 = this->lfo_[LFOIndex::Slow240]->process();

    float32_t fast_0   = this->lfo_[LFOIndex::Fast0  ]->process();
    float32_t fast_120 = this->lfo_[LFOIndex::Fast120]->process();
    float32_t fast_240 = this->lfo_[LFOIndex::Fast240]->process();
      
    float32_t a = this->fullscale_depth_ * 1.0f;
    float32_t b = this->fullscale_depth_ * 0.1f;
    
    float32_t mod_1 = slow_0   * a + fast_0   * b;
    float32_t mod_2 = slow_120 * a + fast_120 * b;
    float32_t mod_3 = slow_240 * a + fast_240 * b;

    float32_t wet = 0.0f;

    // Sum L & R channel to send to chorus line.
    c.read(inL, 1.0f);
    c.write(line_l, 0.0f);
    c.read(inR, 1.0f);
    c.write(line_r, 0.0f);

    c.interpolate(line_l, mod_1 + 1024, 0.33f);
    c.interpolate(line_l, mod_2 + 1024, 0.33f);
    c.interpolate(line_r, mod_3 + 1024, 0.33f);
    c.write(wet, 0.0f);
    outL = wet;
    
    c.interpolate(line_r, mod_1 + 1024, 0.33f);
    c.interpolate(line_r, mod_2 + 1024, 0.33f);
    c.interpolate(line_l, mod_3 + 1024, 0.33f);
    c.write(wet, 0.0f);
    outR = wet;
}

void Orbitone::setRate(float32_t rate)
{
    rate = constrain(rate, 0.0f, 1.0f);
    if(this->lfo_[LFOIndex::Slow0]->getNormalizedFrequency() != rate)
    {
        for(unsigned i = 0; i < 6; ++i)
        {
            this->lfo_[i]->setNormalizedFrequency(rate);
        }
    }
}

float32_t Orbitone::getRate() const
{
    return this->lfo_[LFOIndex::Slow0]->getNormalizedFrequency();
}

void Orbitone::setDepth(float32_t depth)
{
    depth = constrain(depth, 0.0f, 1.0f);
    if(this->depth_ != depth)
    {
        this->depth_ = depth;
        this->fullscale_depth_ = this->depth_ * FULLSCALE_DEPTH_RATIO;
    }
}

float32_t Orbitone::getDepth() const
{
    return this->depth_;
}
