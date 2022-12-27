#include "fx_rack.h"

FXRack::FXRack(float32_t sampling_rate) :
    FX(sampling_rate),
    fx_chain_()
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
    for(FXChain::iterator it = this->fx_chain_.begin(); it != this->fx_chain_.end(); it++)
    {
        delete *it;
    }
    this->fx_chain_.clear();

    delete this->fxTube_;
    delete this->fxChorus_;
    delete this->fxFlanger_;
    delete this->fxOrbitone_;
    delete this->fxPhaser_;
    delete this->fxTapeDelay_;
    delete this->fxShimmerReverb_;
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
    this->fx_chain_.push_back(fx);
}
