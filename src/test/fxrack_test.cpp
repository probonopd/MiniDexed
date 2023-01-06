#include "../fx_rack.h"

#include "../effect_platervbstereo.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <locale>
#include <ctime>
#include <cmath>
#include <random>
#include "wave.h"

using namespace std;

#define FS 44100.0f
#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<float32_t> dist(-1.0f, 1.0f);

float32_t arm_sin_f32(float32_t phase)
{
    return sin(phase);
}

void arm_scale_f32(const float32_t *pSrc, float32_t scale, float32_t *pDst, uint32_t blockSize)
{
    for(unsigned i = 0; i < blockSize; ++i)
    {
        pDst[i] = scale * pSrc[i];
    }
}

void testPlateReverb(unsigned& step)
{
    const unsigned nbRepeats = 4;

    cout << "Step #" << (++step) << " - PlateReverb creation";

    AudioEffectPlateReverb* reverb = new AudioEffectPlateReverb(44100.0f);

    cout << "done" << endl;
    cout << "Step #" << (++step) << " - Setup PlateReverb: ";

    reverb->set_bypass(false);
    reverb->size(0.7f);
    reverb->hidamp(0.5f);
    reverb->lodamp(0.5f);
    reverb->lowpass(0.3f);
    reverb->diffusion(0.65f);
    reverb->level(1.0f);
    

    unsigned size;
    float32_t** samples = readWaveFile("test.wav", size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    cout << "done" << endl;
    cout << "Step #" << (++step) << " - Processing PlateReverb new algo: ";

    unsigned index = 0;
    for(unsigned i = 0; i < nbRepeats; ++i)
    {
        for(unsigned j = 0; j < size; ++j)
        {
            reverb->processSample(samples[0][j], samples[1][j], sampleOutL[index], sampleOutR[index]);
            ++index;
        }
    }
    saveWaveFile("result-new.wav", sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(FS), 16);

    cout << "done" << endl;
    cout << "Step #" << (++step) << " - Processing PlateReverb legacy algo: ";

    unsigned indexOut = 0;
    for (unsigned i = 0; i < nbRepeats; ++i)
    {
        unsigned len = size;
        unsigned indexIn = 0;

        while(len > 0)
        {
            unsigned grainSize = (len < 1024 ? len : 1024);

            reverb->doReverb(samples[0] + indexIn, samples[1] + indexIn, sampleOutL + indexOut, sampleOutR + indexOut, grainSize);

            indexIn += grainSize;
            indexOut += grainSize;
            len -= grainSize;
        }

    }
    saveWaveFile("result-legacy.wav", sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(FS), 16);

    cout << "done" << endl;
    cout << "Step #" << (++step) << " - Cleanup: ";

    delete[] sampleOutL;
    delete[] sampleOutR;
    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;

    cout << "done" << endl;
    cout << "Step #" << (++step) << " - Deleting PlateReverb: ";

    delete reverb;

    cout << "done" << endl;
}

void testLFO(unsigned& step)
{
    cout << "Step #" << (++step) << ": Testing LFO" << endl;

    const float32_t freq = 10.0f;

    LFO lfo(FS, LFO::Waveform::Sine, 0.0f, freq);
    unsigned size = static_cast<unsigned>(8.0f * FS / freq);
    float32_t rate = 0.0f;
    float32_t rate_increment = freq / 2.0f / FS;

    // float32_t* output = new float32_t[size];
    ofstream out("result.csv");

    struct comma_separator : numpunct<char>
    {
        virtual char do_decimal_point() const override { return ','; }
    };

    out.imbue(locale(out.getloc(), new comma_separator));
    out << fixed << showpoint;

    out << "index;LFO" << endl;
    for(unsigned i = 0; i < size; ++i)
    {
        lfo.setNormalizedFrequency(rate);
        out << i << ";" << lfo.process() << endl;
        rate += rate_increment;

        if(rate >= 1.0f || rate <= 0.0f)
        {
            rate_increment *= -1.0f;
        }
    }
}

void testFlutter(unsigned& step)
{
    cout << "Step #" << (++step) << ": Testing JitterGenerator" << endl;

    JitterGenerator jg(FS);
    jg.setSpeed(1.0f);
    jg.setMagnitude(0.1f);

    for (unsigned i = 0; i < 1000; ++i)
    {
        cout << jg.process() << endl;
    }
}

void testSVF(unsigned& step)
{
    float32_t inL, inR;
    float32_t outL, outR;
    StateVariableFilter svf(FS, StateVariableFilter::Type::LPF, 12000.0f);

    cout << "Step #" << (++step) << ": Testing SVF in LPF mode" << endl;
    {
        svf.setFilterType(StateVariableFilter::Type::LPF);
        svf.setCutoff(12000.0f);
        svf.setResonance(0.0f);
        unsigned nbSamples = 0;
        unsigned nbErrors = 0;
        while (nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = dist(gen);
            inR = dist(gen);
            svf.processSample(inL, inR, outL, outR);

            if (abs(outL) > 1.0f)
                nbErrors++;
            if (abs(outR) > 1.0f)
                nbErrors++;
        }
        cout << "nbSamples: " << nbSamples << " -- nbErrors: " << nbErrors << endl;
    }

    cout << "Step #" << (++step) << ": Testing SVF in HPF mode" << endl;
    {
        svf.setFilterType(StateVariableFilter::Type::LPF);
        svf.setCutoff(60.0f);
        svf.setResonance(0.0f);
        unsigned nbSamples = 0;
        unsigned nbErrors = 0;
        while (nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = dist(gen);
            inR = dist(gen);
            svf.processSample(inL, inR, outL, outR);

            if (abs(outL) > 1.0f)
                nbErrors++;
            if (abs(outR) > 1.0f)
                nbErrors++;
        }
        cout << "nbSamples: " << nbSamples << " -- nbErrors: " << nbErrors << endl;
    }
}

enum FXSitch
{
    Tube        = 1 << 0,
    Chorus      = 1 << 1,
    Phaser      = 1 << 2,
    Orbitone    = 1 << 3,
    Flanger     = 1 << 4,
    Delay       = 1 << 5,
    Shimmer     = 1 << 6,
    kNbFX       = 7
};

int scenarii[] = 
{
    0,
    Tube,
    Chorus,
    Phaser,
    Orbitone,
    Flanger,
    Delay,
    Shimmer,
    Tube | Chorus,
    Tube | Phaser,
    Tube | Orbitone,
    Tube | Flanger,
    Tube | Delay,
    Tube | Shimmer,
    Chorus | Phaser,
    Chorus | Orbitone,
    Chorus | Flanger,
    Chorus | Delay,
    Chorus | Shimmer,
    Phaser | Orbitone,
    Phaser | Flanger,
    Phaser | Delay,
    Phaser | Shimmer,
    Orbitone | Flanger,
    Orbitone | Delay,
    Orbitone | Shimmer,
    Flanger | Delay,
    Flanger | Shimmer,
    Delay | Shimmer,
    Tube | Chorus | Phaser,
    Tube | Chorus | Orbitone,
    Tube | Chorus | Flanger,
    Tube | Chorus | Delay,
    Tube | Chorus | Shimmer,
    Tube | Phaser | Orbitone,
    Tube | Phaser | Flanger,
    Tube | Phaser | Delay,
    Tube | Phaser | Shimmer,
    Tube | Orbitone | Flanger,
    Tube | Orbitone | Delay,
    Tube | Orbitone | Shimmer,
    Tube | Flanger | Delay,
    Tube | Flanger | Shimmer,
    Tube | Delay | Shimmer,
    Tube | Chorus | Phaser | Orbitone,
    Tube | Chorus | Phaser | Flanger,
    Tube | Chorus | Phaser | Delay,
    Tube | Chorus | Phaser | Shimmer,
    Tube | Chorus | Orbitone | Flanger,
    Tube | Chorus | Orbitone | Delay,
    Tube | Chorus | Orbitone | Shimmer,
    Tube | Chorus | Flanger | Delay,
    Tube | Chorus | Flanger | Shimmer,
    Tube | Phaser | Orbitone | Flanger,
    Tube | Phaser | Orbitone | Delay,
    Tube | Phaser | Orbitone | Shimmer,
    Tube | Phaser | Flanger | Delay,
    Tube | Phaser | Flanger | Shimmer,
    Tube | Phaser | Delay | Shimmer,
    Tube | Chorus | Phaser | Orbitone | Flanger,
    Tube | Chorus | Phaser | Orbitone | Delay,
    Tube | Chorus | Phaser | Orbitone | Shimmer,
    Tube | Chorus | Phaser | Flanger | Delay,
    Tube | Chorus | Phaser | Flanger | Shimmer,
    Tube | Chorus | Phaser | Delay | Shimmer,
    Tube | Chorus | Orbitone | Flanger | Delay,
    Tube | Chorus | Orbitone | Flanger | Shimmer,
    Tube | Chorus | Flanger | Delay | Shimmer,
    Tube | Phaser | Orbitone | Flanger | Delay,
    Tube | Phaser | Orbitone | Flanger | Shimmer,
    Tube | Phaser | Orbitone | Delay | Shimmer,
    Tube | Orbitone | Flanger | Delay | Shimmer,
    Tube | Chorus | Phaser | Orbitone | Flanger | Delay,
    Tube | Chorus | Phaser | Orbitone | Flanger | Shimmer,
    Tube | Chorus | Phaser | Orbitone | Delay | Shimmer,
    Tube | Chorus | Phaser | Flanger | Delay | Shimmer,
    Tube | Chorus | Orbitone | Flanger | Delay | Shimmer,
    Tube | Phaser | Orbitone | Flanger | Delay | Shimmer,
    Tube | Chorus | Phaser | Orbitone | Flanger | Delay | Shimmer,
    -1
};

#define Active(fxSwitch, FxID) ((fxSwitch & FxID) == FxID)

string getScenarioName(int scenario)
{
    stringstream ss;

    bool fxTube = Active(scenario, FXSitch::Tube);
    bool fxChorus = Active(scenario, FXSitch::Chorus);
    bool fxPhaser = Active(scenario, FXSitch::Phaser);
    bool fxOrbitone = Active(scenario, FXSitch::Orbitone);
    bool fxFlanger = Active(scenario, FXSitch::Flanger);
    bool fxDelay = Active(scenario, FXSitch::Delay);
    bool fxShimmer = Active(scenario, FXSitch::Shimmer);
    bool first = true;

    ss << "[ ";

    if(fxTube) 
    {
        if(!first) ss << ", ";
        ss << "Tube";
        first = false;
    }

    if(fxChorus) 
    {
        if(!first) ss << ", ";
        ss << "Chorus";
        first = false;
    }

    if(fxPhaser) 
    {
        if(!first) ss << ", ";
        ss << "Phaser";
        first = false;
    }

    if(fxOrbitone) 
    {
        if(!first) ss << ", ";
        ss << "Orbitone";
        first = false;
    }

    if(fxFlanger) 
    {
        if(!first) ss << ", ";
        ss << "Flanger";
        first = false;
    }

    if(fxDelay) 
    {
        if(!first) ss << ", ";
        ss << "Delay";
        first = false;
    }

    if(fxShimmer) 
    {
        if(!first) ss << ", ";
        ss << "Shimmer";
        first = false;
    }

    ss << " ]";

    return ss.str();
}

void setupRack(unsigned& step, FXRack* rack)
{
    cout << "Step #" << (++step) << ": Set FXRack parameters" << endl;
    rack->setWetLevel(1.0f);

    rack->getTube()->setWetLevel(0.25f);
    rack->getTube()->setOverdrive(0.25f);

    rack->getChorus()->setWetLevel(0.5f);
    rack->getChorus()->setRate(0.4f);
    rack->getChorus()->setDepth(0.5f);
    
    rack->getPhaser()->setWetLevel(1.0f);
    rack->getPhaser()->setRate(0.1f);
    rack->getPhaser()->setDepth(1.0f);
    rack->getPhaser()->setFeedback(0.5f);
    rack->getPhaser()->setNbStages(12);

    rack->getOrbitone()->setWetLevel(0.8f);
    rack->getOrbitone()->setRate(0.4f);
    rack->getOrbitone()->setDepth(0.5f);

    rack->getFlanger()->setWetLevel(0.5f);
    rack->getFlanger()->setRate(0.03f);
    rack->getFlanger()->setDepth(0.75f);
    rack->getFlanger()->setFeedback(0.5f);

    rack->getDelay()->setWetLevel(0.6f);
    rack->getDelay()->setLeftDelayTime(0.075f);
    rack->getDelay()->setLeftDelayTime(0.05f);
    rack->getDelay()->setFeedbak(0.5f);

    rack->getShimmerReverb()->setWetLevel(0.5f);
    rack->getShimmerReverb()->setInputGain(0.35f);
    rack->getShimmerReverb()->setTime(0.89f);
    rack->getShimmerReverb()->setDiffusion(0.75f);
    rack->getShimmerReverb()->setLP(0.8f);
}

void activateRackFXUnitScenario(unsigned& step, FXRack* rack, int scenario)
{
    rack->getTube()->setEnable(Active(scenario, FXSitch::Tube));
    rack->getChorus()->setEnable(Active(scenario, FXSitch::Chorus));
    rack->getPhaser()->setEnable(Active(scenario, FXSitch::Phaser));
    rack->getOrbitone()->setEnable(Active(scenario, FXSitch::Orbitone));
    rack->getFlanger()->setEnable(Active(scenario, FXSitch::Flanger));
    rack->getDelay()->setEnable(Active(scenario, FXSitch::Delay));
    rack->getShimmerReverb()->setEnable(Active(scenario, FXSitch::Shimmer));
}

void testReset(unsigned& step)
{
    FXRack *rack = new FXRack(44100.0f);
    rack->setEnable(true);

    setupRack(step, rack);

    unsigned i = 0;
    while(true)
    {
        int fxSwitch = scenarii[i];
        if(fxSwitch == -1)
        {
            break;
        }

        activateRackFXUnitScenario(step, rack, fxSwitch);

        cout << "Step #" << (++step) << ": Reset for scenario #" << i << " => " << getScenarioName(fxSwitch) << ": ";
        rack->reset();
        cout << "done" << endl;

        ++i;
    }

    delete rack;
}

void testProcessing(unsigned& step)
{
    const unsigned nbRepeats = 1;
    unsigned size;
    float32_t** samples = readWaveFile("test.wav", size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    FXRack *rack = new FXRack(44100.0f);
    rack->setEnable(true);

    setupRack(step, rack);

    unsigned i = 0;
    while(true)
    {
        int fxSwitch = scenarii[i];
        if(fxSwitch == -1)
        {
            break;
        }

        activateRackFXUnitScenario(step, rack, fxSwitch);

        string name = getScenarioName(fxSwitch);
        cout << "Step #" << (++step) << ": Processing for scenario #" << i << " => " << name << ": ";

        for (unsigned i = 0; i < nbRepeats; ++i)
        {
            rack->process(samples[0], samples[1], sampleOutL + i * size, sampleOutR + i * size, size);
        }

        // stringstream ss;
        // ss << "result " << name << ".wav";
        // saveWaveFile(ss.str(), sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(FS), 16);

        cout << "done" << endl;

        ++i;
    }

    delete rack;

    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;
    delete[] sampleOutL;
    delete[] sampleOutR;
}

int main()
{
    unsigned step = 0;

    testLFO(step);
    testFlutter(step);
    testSVF(step);
    testPlateReverb(step);
    testReset(step);
    testProcessing(step);

    return 0;
}
