#include "fx_tube.h"

#include <cmath>

Tube::Tube(float32_t samplingRate, float32_t curve, float32_t bias) :
    FXElement(samplingRate),
    TubeCurve(curve),
    TubeBias(bias)
{
}

Tube::~Tube()
{
}

void Tube::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    float32_t absInL = abs(inL);

    float32_t coeff = this->TubeCurve + this->getOverdrive();

    if(absInL > this->TubeBias) 
    {
        outL = coeff * (absInL - this->TubeBias) / (1.0f - this->TubeBias);
    }
    else
    {
        outL = coeff * absInL / (1.0f + this->TubeBias * absInL);
    }

    if(inL < 0.0f)
    {
        outL = -outL;
    }

    float32_t absInR = abs(inR);
    if(absInR > this->TubeBias) 
    {
        outR = coeff * (absInR - this->TubeBias) / (1.0f - this->TubeBias);
    }
    else
    {
        outR = coeff * absInR / (1.0f + this->TubeBias * absInR);
    }

    if(inR < 0.0f)
    {
        outR = -outR;
    }
}

void Tube::setOverdrive(float32_t overdrive)
{
    this->overdrive_ = constrain(overdrive, 0.0f, 1.0f);
}

float32_t Tube::getOverdrive() const
{
    return this->overdrive_;
}
