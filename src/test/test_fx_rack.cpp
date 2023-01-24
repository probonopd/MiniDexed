#include <gtest/gtest.h>
#include <cmath>

#include "test_fx_helper.h"
#include "wave.h"

#include "../fx_rack.h"
#include "../effect_platervbstereo.h"

using namespace std;

#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

void setupRack(FXRack* rack)
{
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
    rack->getDelay()->setLeftDelayTime(0.15f);
    rack->getDelay()->setLeftDelayTime(0.2f);
    rack->getDelay()->setFeedback(0.35f);
    rack->getDelay()->setFlutterRate(0.15f);
    rack->getDelay()->setFlutterAmount(0.75f);

    rack->getShimmerReverb()->setWetLevel(0.6f);
    rack->getShimmerReverb()->setInputGain(0.55f);
    rack->getShimmerReverb()->setTime(0.89f);
    rack->getShimmerReverb()->setDiffusion(0.75f);
    rack->getShimmerReverb()->setLP(0.8f);
}

void activateRackFXUnitScenario(FXRack* rack, int scenario)
{
    rack->getTube()->setEnable(Active(scenario, FXSwitch::FX__Tube));
    rack->getChorus()->setEnable(Active(scenario, FXSwitch::FX__Chorus));
    rack->getPhaser()->setEnable(Active(scenario, FXSwitch::FX__Phaser));
    rack->getOrbitone()->setEnable(Active(scenario, FXSwitch::FX__Orbitone));
    rack->getFlanger()->setEnable(Active(scenario, FXSwitch::FX__Flanger));
    rack->getDelay()->setEnable(Active(scenario, FXSwitch::FX__Delay));
    rack->getShimmerReverb()->setEnable(Active(scenario, FXSwitch::FX__ShimmerReverb));
}

TEST_P(FXScenarioTest, FXRackResetAllScenarios)
{
    FXRack *rack = new FXRack(SAMPLING_FREQUENCY);
    rack->setEnable(true);
    setupRack(rack);

    int fxSwitch = GetParam();
    activateRackFXUnitScenario(rack, fxSwitch);

    rack->reset();
    delete rack;
}

TEST_P(FXScenarioTest, ScenarioProcessing)
{
    const unsigned nbRepeats = 1;
    unsigned size;
    float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    FXRack *rack = new FXRack(SAMPLING_FREQUENCY);
    rack->setEnable(true);

    setupRack(rack);

    rack->reset();
    int fxSwitch = GetParam();

    activateRackFXUnitScenario(rack, fxSwitch);

    string name = getScenarioName(fxSwitch);

    for(unsigned i = 0; i < nbRepeats; ++i)
    {
        rack->process(samples[0], samples[1], sampleOutL + i * size, sampleOutR + i * size, size);
    }

    stringstream ss;
    ss << "result-fx-rack" << name << ".wav";
    saveWaveFile(getResultFile(ss.str()), sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    delete rack;

    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;
    delete[] sampleOutL;
    delete[] sampleOutR;
}

INSTANTIATE_TEST_SUITE_P(FXRack, FXScenarioTest, testing::Range(0, 1 << (FXSwitch::FX__ShimmerReverb + 1)));
