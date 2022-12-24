#include "fx_rack.h"

FXUnit::FXUnit(float32_t sampling_rate, FXElement& fx, float32_t wet_level) :
    FXElement(sampling_rate),
    fx_(fx)
{
    this->setWetLevel(wet_level);
}

FXUnit::~FXUnit()
{
}

void FXUnit::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    this->fx_.processSample(inL, inR, outL, outR);

    // Mix wet and dry signals
    outL = this->getWetLevel() * outL + (1.0f - this->getWetLevel()) * inL;
    outR = this->getWetLevel() * outR + (1.0f - this->getWetLevel()) * inR;
}

void FXUnit::setWetLevel(float32_t wet_level)
{
    this->wet_level_ = constrain(wet_level, 0.0f, 1.0f);
}

float32_t FXUnit::getWetLevel() const
{
    return this->wet_level_;
}

FXRack::FXRack(float32_t sampling_rate) :
    FX(sampling_rate),
    fx_chain_()
{
    this->registerFX(new Phaser(sampling_rate));
    this->registerFX(new TapeDelay(sampling_rate));
    this->registerFX(new ShimmerReverb(sampling_rate));
}

FXRack::~FXRack()
{
    for(FXChain::iterator it = this->fx_chain_.begin(); it != this->fx_chain_.end(); it++)
    {
        delete *it;
    }
    this->fx_chain_.clear();
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

        for(FXChain::iterator it = this->fx_chain_.begin(); it != this->fx_chain_.end(); it++)
        {
            (*it)->processSample(sampleInL, sampleInR, sampleOutL, sampleOutR);

            sampleInL = sampleOutL;
            sampleInR = sampleOutR;
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

void FXRack::registerFX(FXElement* fx)
{
    this->fx_chain_.push_back(new FXUnit(this->getSamplingRate(), *fx));
}
