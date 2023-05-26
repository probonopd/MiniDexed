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

FXElement::FXElement(float32_t sampling_rate, float32_t output_level_corrector) :
    FXBase(sampling_rate),
    OutputLevelCorrector(output_level_corrector),
    bypass_fx_process_(false)
{
}

FXElement::~FXElement()
{
}

void FXElement::bypassFXProcess(bool bypass)
{
    this->bypass_fx_process_ = bypass;
}

bool FXElement::bypassFXProcess() const
{
    return this->bypass_fx_process_;
}

FX::FX(float32_t sampling_rate) :
    FXBase(sampling_rate)
{
}

FX::~FX()
{
}
