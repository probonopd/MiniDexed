#include "fx_tube.h"

#include <cmath>

Tube::Tube(float32_t samplingRate) :
    FXElement(samplingRate),
    overdrive_(0.0f),
    saturation_(0.0f)
{
    this->setOverdrive(0.0f);
}

Tube::~Tube()
{
}

void Tube::reset()
{
    // does nothing
}

void Tube::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = softSaturator2(inL, this->saturation_);
    outR = softSaturator2(inR, this->saturation_);
}

void Tube::setOverdrive(float32_t overdrive)
{
    this->overdrive_ = constrain(overdrive, 0.0f, 1.0f);
    this->saturation_ = 2.0f * this->overdrive_;
}

float32_t Tube::getOverdrive() const
{
    return this->overdrive_;
}
