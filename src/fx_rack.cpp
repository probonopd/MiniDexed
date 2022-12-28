#include "fx_rack.h"

#include <cassert>

FXRack::FXRack(float32_t sampling_rate, bool enable, float32_t wet) :
    FX(sampling_rate),
    FXElement(sampling_rate),
    enable_(enable),
    wet_level_(wet),
    fx_chain_(sampling_rate)
{
    this->fxTube_ = new FXUnit<Tube>(sampling_rate);
    this->fxChorus_ = new FXUnit<Chorus>(sampling_rate);
    this->fxFlanger_ = new FXUnit<Flanger>(sampling_rate);
    this->fxOrbitone_ = new FXUnit<Orbitone>(sampling_rate);
    this->fxPhaser_ = new FXUnit<Phaser>(sampling_rate);
    this->fxTapeDelay_ = new FXUnit<TapeDelay>(sampling_rate);
    this->fxShimmerReverb_ = new FXUnit<ShimmerReverb>(sampling_rate);

    this->registerFX(this->fxTube_);
    this->registerFX(this->fxChorus_);
    this->registerFX(this->fxFlanger_);
    this->registerFX(this->fxOrbitone_);
    this->registerFX(this->fxPhaser_);
    this->registerFX(this->fxTapeDelay_);
    this->registerFX(this->fxShimmerReverb_);
}

FXRack::~FXRack()
{
    this->fx_chain_.clear();

    delete this->fxTube_;
    delete this->fxChorus_;
    delete this->fxFlanger_;
    delete this->fxOrbitone_;
    delete this->fxPhaser_;
    delete this->fxTapeDelay_;
    delete this->fxShimmerReverb_;
}

void FXRack::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    for(FXChain::iterator it = this->fx_chain_.begin(); it != this->fx_chain_.end(); it++)
    {
        (*it)->processSample(inL, inR, outL, outR);

        inL = outL;
        inR = outR;
    }
}

void FXRack::process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples)
{
    float32_t sampleInL;
    float32_t sampleInR;
    float32_t sampleOutL;
    float32_t sampleOutR;

    for(unsigned i = 0; i < nSamples; ++i)
    {
        sampleInL = *left_input;
        sampleInR = *right_input;
        sampleOutL = 0.0f;
        sampleOutR = 0.0f;

        if(this->isEnable()) 
        {
            this->processSample(sampleInL, sampleInR, sampleOutL, sampleOutR);
        }
        else
        {
            sampleOutL = sampleInL;
            sampleOutR = sampleInR;
        }

        *left_output = sampleOutL;
        *right_output = sampleOutR;

        // Move inputs by 1 sample
        ++left_input;
        ++right_input;

        // Move outputs by 1 sample
        ++left_input;
        ++right_input;
    }
}

void FXRack::setEnable(bool enable)
{
    this->enable_ = enable;
}

bool FXRack::isEnable() const
{
    return this->enable_;
}

void FXRack::setWetLevel(float32_t wet_level)
{
    this->wet_level_ = constrain(wet_level, 0.0f, 1.0f);
}

float32_t FXRack::getWetLevel() const
{
    return this->wet_level_;
}

void FXRack::registerFX(FXElement* fx)
{
    assert(fx);
    this->fx_chain_.push_back(fx);
}

FXUnit<Tube>* FXRack::getTube()
{
    return this->fxTube_;
}

FXUnit<Chorus>* FXRack::getChorus()
{
    return this->fxChorus_;
}

FXUnit<Flanger>* FXRack::getFlanger()
{
    return this->fxFlanger_;
}

FXUnit<Orbitone>* FXRack::getOrbitone()
{
    return this->fxOrbitone_;
}

FXUnit<Phaser>* FXRack::getPhaser()
{
    return this->fxPhaser_;
}

FXUnit<TapeDelay>* FXRack::getTapeDelay()
{
    return this->fxTapeDelay_;
}

FXUnit<ShimmerReverb>* FXRack::getShimmerReverb()
{
    return this->fxShimmerReverb_;
}
