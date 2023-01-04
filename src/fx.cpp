#include "fx.h"

FXBase::FXBase(float32_t sampling_rate) :
    SamplingRate(sampling_rate)
{
}

FXBase::~FXBase()
{
}

float32_t FXBase::getSamplingRate() const
{
    return this->SamplingRate;
}

FXElement::FXElement(float32_t sampling_rate) :
    FXBase(sampling_rate)
{
}

FXElement::~FXElement()
{
}


FX::FX(float32_t sampling_rate) :
    FXBase(sampling_rate)
{
}

FX::~FX()
{
}
