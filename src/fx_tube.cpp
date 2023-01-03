#include "fx_tube.h"

#include <cmath>

Tube::Tube(float32_t samplingRate) :
    FXElement(samplingRate)
{
    this->setOverdrive(0.0f);
}

Tube::~Tube()
{
}

void Tube::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = softSaturate(inL, this->threshold_);
    outR = softSaturate(inR, this->threshold_);
}

void Tube::setOverdrive(float32_t overdrive)
{
    this->overdrive_ = constrain(overdrive, 0.0f, 1.0f);
    this->threshold_ = 1.0f - this->overdrive_;
}

float32_t Tube::getOverdrive() const
{
    return this->overdrive_;
}
