#include "../fx_rack.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <locale>
#include <ctime>
#include <random>
#include "wave.h"

using namespace std;

#define FS 44100.0f
#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float32_t> dist(-1.0f, 1.0f);

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

    struct comma_separator : std::numpunct<char>
    {
        virtual char do_decimal_point() const override { return ','; }
    };

    out.imbue(std::locale(out.getloc(), new comma_separator));
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

            if (std::abs(outL) > 1.0f)
                nbErrors++;
            if (std::abs(outR) > 1.0f)
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

            if (std::abs(outL) > 1.0f)
                nbErrors++;
            if (std::abs(outR) > 1.0f)
                nbErrors++;
        }
        cout << "nbSamples: " << nbSamples << " -- nbErrors: " << nbErrors << endl;
    }
}

void testFXRack(unsigned& step)
{
    cout << "Step #" << (++step) << ": Intanciation FXRack" << endl;
    FXRack *rack = new FXRack(44100.0f);

    cout << "Step #" << (++step) << ": Test preparation" << endl;
    rack->setEnable(true);
    rack->setWetLevel(1.0f);

    rack->getTube()->setEnable(false);
    rack->getTube()->setWetLevel(1.0f);
    rack->getTube()->setOverdrive(1.0f);

    rack->getChorus()->setEnable(false);
    rack->getChorus()->setWetLevel(0.5f);
    rack->getChorus()->setRate(0.4f);
    rack->getChorus()->setDepth(0.5f);
    
    rack->getPhaser()->setEnable(false);

    rack->getOrbitone()->setEnable(true);
    rack->getOrbitone()->setWetLevel(0.8f);
    rack->getOrbitone()->setRate(0.4f);
    rack->getOrbitone()->setDepth(0.5f);

    rack->getFlanger()->setEnable(false);

    rack->getDelay()->setEnable(false);
    rack->getDelay()->setWetLevel(0.6f);
    rack->getDelay()->setLeftDelayTime(0.075f);
    rack->getDelay()->setLeftDelayTime(0.05f);
    rack->getDelay()->setFeedbak(0.5f);

    rack->getShimmerReverb()->setEnable(false);
    rack->getShimmerReverb()->setWetLevel(0.7f);
    rack->getShimmerReverb()->setInputGain(0.45f);
    rack->getShimmerReverb()->setTime(0.89f);
    rack->getShimmerReverb()->setDiffusion(0.75f);
    rack->getShimmerReverb()->setLP(0.8f);

    const unsigned nSamples = 1;
    float32_t inSamples[2][nSamples];
    float32_t outSamples[2][nSamples];

    for (unsigned i = 0; i < nSamples; ++i)
    {
        inSamples[0][i] = dist(gen);
        inSamples[1][i] = dist(gen);
    }

    memset(outSamples[0], 0, nSamples * sizeof(float32_t));
    memset(outSamples[1], 0, nSamples * sizeof(float32_t));

    cout << "Step #" << (++step) << ": Run test" << endl;
    rack->process(inSamples[0], inSamples[1], outSamples[0], outSamples[1], nSamples);

    cout << "Step #" << (++step) << ": Render results" << endl;
    for (unsigned i = 0; i < nSamples; ++i)
    {
        std::cout << "#" << i << " " << inSamples[0][i] << "   --> " << outSamples[0][i] << " = " << ((outSamples[0][i] - inSamples[0][i]) * 100.0f / inSamples[0][i]) << "%" << std::endl;
    }

    unsigned nbRepeats = 4;

    unsigned size;
    float32_t** samples = readWaveFile("test.wav", size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    for (unsigned i = 0; i < nbRepeats; ++i)
    {
        rack->process(samples[0], samples[1], sampleOutL + i * size, sampleOutR + i * size, size);
    }

    saveWaveFile("result.wav", sampleOutL, sampleOutR, nbRepeats * size, 44100, 16);

    delete[] sampleOutL;
    delete[] sampleOutR;
    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;

    cout << "Step #" << (++step) << ": Test cleanup" << endl;
    delete rack;
}

int main()
{
    unsigned step = 0;

    testLFO(step);
    // testFlutter(step);
    // testSVF(step);
    // testFXRack(step);

    return 0;
}
