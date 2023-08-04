#include "fx_diffuser.h"

#include <cmath>
#include <algorithm>

#define TAIL , -1

Diffuser::Diffuser(float32_t sampling_frequency) : 
    FXElement(sampling_frequency),
    engine_(sampling_frequency)
{
}

Diffuser::~Diffuser()
{
}

void Diffuser::reset()
{
    this->engine_.reset();
}

void Diffuser::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    typedef Engine::Reserve<126,
            Engine::Reserve<180,
            Engine::Reserve<269,
            Engine::Reserve<444,
            Engine::Reserve<151,
            Engine::Reserve<205,
            Engine::Reserve<245,
            Engine::Reserve<405> > > > > > > > Memory;
    Engine::DelayLine<Memory, 0> apl1;
    Engine::DelayLine<Memory, 1> apl2;
    Engine::DelayLine<Memory, 2> apl3;
    Engine::DelayLine<Memory, 3> apl4;
    Engine::DelayLine<Memory, 4> apr1;
    Engine::DelayLine<Memory, 5> apr2;
    Engine::DelayLine<Memory, 6> apr3;
    Engine::DelayLine<Memory, 7> apr4;
    Engine::Context c;

    const float32_t kap = 0.625f;
    float wet = 0.0f;

    engine_.start(&c);

    c.load(inL);
    c.read(apl1 TAIL, kap);
    c.writeAllPass(apl1, -kap);
    c.read(apl2 TAIL, kap);
    c.writeAllPass(apl2, -kap);
    c.read(apl3 TAIL, kap);
    c.writeAllPass(apl3, -kap);
    c.read(apl4 TAIL, kap);
    c.writeAllPass(apl4, -kap);
    c.writeAndLoad(wet, 0.0f);
    outL = wet;

    c.load(inR);
    c.read(apr1 TAIL, kap);
    c.writeAllPass(apr1, -kap);
    c.read(apr2 TAIL, kap);
    c.writeAllPass(apr2, -kap);
    c.read(apr3 TAIL, kap);
    c.writeAllPass(apr3, -kap);
    c.read(apr4 TAIL, kap);
    c.writeAllPass(apr4, -kap);
    c.writeAndLoad(wet, 0.0f);
    outR = wet;
}
