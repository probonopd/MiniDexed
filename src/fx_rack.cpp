#include "fx_rack.h"

FXUnit::FXUnit(float32_t sampling_rate, FX& fx, float32_t wet_level) :
    FX(sampling_rate),
    fx_(fx)
{
    this->setWetLevel(wet_level);
}

FXUnit::~FXUnit()
{
}

void FXUnit::process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples)
{
    this->fx_.process(left_input, right_input, left_output, right_output, nSamples);

    for(unsigned i = 0; i < nSamples; ++i)
    {
        // Mix wet and dry signals
        *left_output = this->getWetLevel() * *left_output + (1.0f - this->getWetLevel()) * *left_input;
        *right_output = this->getWetLevel() * *right_output + (1.0f - this->getWetLevel()) * *left_input;
        
        // Move to next input sample
        ++left_input;
        ++right_input;

        // Move to next output sample
        ++left_output;
        ++right_output;
    }
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
    for(FXChain::iterator it = this->fx_chain_.begin(); it != this->fx_chain_.end(); it++)
    {
        (*it)->process(left_input, right_input, left_output, right_output, nSamples);

        left_input = left_output;
        right_input = right_output;
    }
}

void FXRack::registerFX(FX* fx)
{
    this->fx_chain_.push_back(new FXUnit(this->getSamplingRate(), *fx));
}
