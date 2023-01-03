#include "../fx_rack.h"

#include <iostream>
#include <ctime>
#include <random>
#include "wave.h"

using namespace std;

#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

void testFlutter()
{
    JitterGenerator jg(44100.0f);
    jg.setSpeed(1.0f);
    jg.setMagnitude(0.1f);

    for(unsigned i = 0; i < 1000; ++i)
    {
        cout << jg.process() << endl;
    }
}

int main()
{
    // testFlutter();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float32_t> dist(-1.0f, 1.0f);

    unsigned step = 0;
    float32_t inL, inR;
    float32_t outL, outR;
    StateVariableFilter svf(44100.0f, StateVariableFilter::Type::LPF, 12000.0f);
    cout << "Step #" << (++step) << ": Testing SVF in LPF mode" << endl;
    {
        svf.setFilterType(StateVariableFilter::Type::LPF);
        svf.setCutoff(12000.0f);
        svf.setResonance(0.0f);
        unsigned nbSamples = 0;
        unsigned nbErrors = 0;
        while(nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = dist(gen);
            inR = dist(gen);
            svf.processSample(inL, inR, outL, outR);

            if(std::abs(outL) > 1.0f) nbErrors++;
            if(std::abs(outR) > 1.0f) nbErrors++;
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
        while(nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = dist(gen);
            inR = dist(gen);
            svf.processSample(inL, inR, outL, outR);

            if(std::abs(outL) > 1.0f) nbErrors++;
            if(std::abs(outR) > 1.0f) nbErrors++;
        }    
        cout << "nbSamples: " << nbSamples << " -- nbErrors: " << nbErrors << endl;
    }

    cout << "Step #" << (++step) << ": Intanciation FXRack" << endl;
    FXRack* rack = new FXRack(44100.0f);

    cout << "Step #" << (++step) << ": Test preparation" << endl;
    rack->setEnable(true);
    rack->setWetLevel(1.0f);

    rack->getTube()->setEnable(false);
    rack->getChorus()->setEnable(false);
    rack->getPhaser()->setEnable(false);
    rack->getOrbitone()->setEnable(true);
    rack->getFlanger()->setEnable(false);
    rack->getTapeDelay()->setEnable(false);
    // rack->getTapeDelay()->setWetLevel(0.6f);
    // rack->getTapeDelay()->setLeftDelayTime(0.1f);
    // rack->getTapeDelay()->setLeftDelayTime(0.05f);
    // rack->getTapeDelay()->setFlutterLevel(0.25f);
    // rack->getTapeDelay()->setFeedbak(0.5f);
    rack->getShimmerReverb()->setEnable(true);
    rack->getShimmerReverb()->setWetLevel(60);
    rack->getShimmerReverb()->setDecay(30);
    rack->getShimmerReverb()->setDiffusion(80);
    rack->getShimmerReverb()->setPitchShift(99);

    const unsigned nSamples = 3000;
    float32_t inSamples[2][nSamples];
    float32_t outSamples[2][nSamples];

    for(unsigned i = 0; i < nSamples; ++i)
    {
        inSamples[0][i] = dist(gen);
        inSamples[1][i] = dist(gen);
    }

    memset(outSamples[0], 0, nSamples * sizeof(float32_t));
    memset(outSamples[1], 0, nSamples * sizeof(float32_t));

    cout << "Step #" << (++step) << ": Run test" << endl;
    rack->process(inSamples[0], inSamples[1], outSamples[0], outSamples[1], nSamples);

    cout << "Step #" << (++step) << ": Render results" << endl;
    for(unsigned i = 0; i < nSamples; ++i)
    {
        std::cout << "#" << i << " " << inSamples[0][i] << "   --> " << outSamples[0][i] << " = " << ((outSamples[0][i] - inSamples[0][i]) * 100.0f / inSamples[0][i]) << "%" << std::endl;
    }


    unsigned size;
    float32_t** samples = readWaveFile("test.wav", size);
    float32_t* sampleOutL = new float32_t[size];
    float32_t* sampleOutR = new float32_t[size];
    rack->process(samples[0], samples[1], sampleOutL, sampleOutR, size);

    playSound(sampleOutL, sampleOutR, size, 44100, 16);

    cout << "Step #" << (++step) << ": Test cleanup" << endl;
    delete rack;

    return 0;
}