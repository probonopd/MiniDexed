#include "fx_reverberator.h"

#define TAIL , -1

Reverberator::Reverberator(float32_t sampling_rate) : 
    FXElement(sampling_rate),
    engine_(sampling_rate),
    input_gain_(-1.0f),
    reverb_time_(0.0f),
    diffusion_(-1.0f),
    lp_(-1.0f),
    lp_decay_1_(0.0f),
    lp_decay_2_(0.0f)
{
    this->engine_.setLFOFrequency(Engine::LFOIndex::LFO_1, 0.5f);
    this->engine_.setLFOFrequency(Engine::LFOIndex::LFO_2, 0.3f);
    
    this->setInputGain(1.0f);
    this->setTime(0.7f);
    this->setDiffusion(0.625f);
    this->setLP(0.7f);

    this->reset();
}

Reverberator::~Reverberator()
{
}

void Reverberator::reset()
{
    this->engine_.reset();
    this->lp_decay_1_ = 0.0f;
    this->lp_decay_2_ = 0.0f;
}

void Reverberator::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // This is the Griesinger topology described in the Dattorro paper
    // (4 AP diffusers on the input, then a loop of 2x 2AP+1Delay).
    // Modulation is applied in the loop of the first diffuser AP for additional
    // smearing; and to the two long delays for a slow shimmer/chorus effect.
    typedef Engine::Reserve< 113,
            Engine::Reserve< 162,
            Engine::Reserve< 241,
            Engine::Reserve< 399,
            Engine::Reserve<1653,
            Engine::Reserve<2038,
            Engine::Reserve<3411,
            Engine::Reserve<1913,
            Engine::Reserve<1663,
            Engine::Reserve<4782> > > > > > > > > > Memory;
    Engine::DelayLine<Memory, 0> ap1;
    Engine::DelayLine<Memory, 1> ap2;
    Engine::DelayLine<Memory, 2> ap3;
    Engine::DelayLine<Memory, 3> ap4;
    Engine::DelayLine<Memory, 4> dap1a;
    Engine::DelayLine<Memory, 5> dap1b;
    Engine::DelayLine<Memory, 6> del1;
    Engine::DelayLine<Memory, 7> dap2a;
    Engine::DelayLine<Memory, 8> dap2b;
    Engine::DelayLine<Memory, 9> del2;
    Engine::Context c;

    const float32_t kap = this->diffusion_;
    const float32_t klp = this->lp_;
    const float32_t krt = this->reverb_time_;
    const float32_t gain = this->input_gain_;

    float32_t lp_1 = this->lp_decay_1_;
    float32_t lp_2 = this->lp_decay_2_;

    float32_t wet = 0.0f;
    float32_t apout = 0.0f;
    engine_.start(&c);
    
    // Smear AP1 inside the loop.
    c.interpolate(ap1, 10.0f, Engine::LFOIndex::LFO_1, 60.0f, 1.0f);
    c.writeAndLoad(ap1, 100, 0.0f);
    c.read(inL + inR, gain);

    // Diffuse through 4 allpasses.
    c.read(ap1 TAIL, kap);
    c.writeAllPass(ap1, -kap);
    c.read(ap2 TAIL, kap);
    c.writeAllPass(ap2, -kap);
    c.read(ap3 TAIL, kap);
    c.writeAllPass(ap3, -kap);
    c.read(ap4 TAIL, kap);
    c.writeAllPass(ap4, -kap);
    c.write(apout);
      
    // Main reverb loop.
    c.load(apout);
    c.interpolate(del2, 4680.0f, Engine::LFOIndex::LFO_2, 100.0f, krt);
    c.lp(lp_1, klp);
    c.read(dap1a TAIL, -kap);
    c.writeAllPass(dap1a, kap);
    c.read(dap1b TAIL, kap);
    c.writeAllPass(dap1b, -kap);
    c.write(del1, 2.0f);
    c.writeAndLoad(wet, 0.0f);

    outL = wet;

    c.load(apout);
    c.read(del1 TAIL, krt);
    c.lp(lp_2, klp);
    c.read(dap2a TAIL, kap);
    c.writeAllPass(dap2a, -kap);
    c.read(dap2b TAIL, -kap);
    c.writeAllPass(dap2b, kap);
    c.write(del2, 2.0f);
    c.writeAndLoad(wet, 0.0f);

    outR = wet;

    this->lp_decay_1_ = lp_1;
    this->lp_decay_2_ = lp_2;
}

void Reverberator::setInputGain(float32_t gain)
{
    this->input_gain_ = constrain(gain, 0.0f, 1.0f);
}

float32_t Reverberator::getInputGain() const
{
    return this->input_gain_;
}

void Reverberator::setTime(float32_t time)
{
    this->reverb_time_ = constrain(time, 0.0f, 1.0f);
}

float32_t Reverberator::getTime() const
{
    return this->reverb_time_;
}

void Reverberator::setDiffusion(float32_t diffusion)
{
    this->diffusion_ = constrain(diffusion, 0.0f, 1.0f);
}

float32_t Reverberator::getDiffusion() const
{
    return this->diffusion_;
}

void Reverberator::setLP(float32_t lp)
{
    this->lp_ = constrain(lp, 0.0f, 1.0f);
}

float32_t Reverberator::getLP() const
{
    return this->lp_;
}
