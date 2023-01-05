#include "fx_dry.h"

Dry::Dry(float32_t samplingRate) :
    FXElement(samplingRate)
{
}

Dry::~Dry()
{
}

void Dry::reset()
{
    // does nothing
}

void Dry::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = inL;
    outR = inR;
}
