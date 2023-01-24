#include "../mixing_console_constants.h"
#include "../mixing_console.hpp"

#include <iostream>

using namespace std;

typedef MixingConsole<8>  Mixer;

int main()
{
    Mixer* mixer = new Mixer(44100.0f, 4);
    // DUMP2(mixer, cout, "Post creation");

    mixer->reset();
    DUMP2(mixer, cout, "Post creation");

    mixer->setChannelLevel(0, 1.0f);
    // DUMP2(mixer, cout, "Post setChannelLevel");

    mixer->setPan(0, 0.5f);
    // DUMP2(mixer, cout, "Post setPan");

    float32_t samples[] = {0.0f, 0.0f, 0.0f, 0.0f};
    mixer->setInputSampleBuffer(0, samples);
    // DUMP2(mixer, cout, "Post setInputSampleBuffer");

    mixer->setSendLevel(0, MixerOutput::MainOutput, 1.0f);
    // DUMP2(mixer, cout, "Post setSendLevel - full dry");

    float32_t outL[4];
    float32_t outR[4];
    mixer->process(outL, outR);
    DUMP2(mixer, cout, "Post process");
    cout.precision(5);
    cout << std::fixed;
    cout << "+ outL: " << outL[0] << " - " << outL[1] << endl;
    cout << "+ outR: " << outR[0] << " - " << outR[1] << endl;

    mixer->setSendLevel(0, MixerOutput::FX_Tube, 1.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Delay, 1.0f);
    mixer->setSendLevel(0, MixerOutput::FX_PlateReverb, 1.0f);

    mixer->setReturnLevel(MixerOutput::FX_Tube, MixerOutput::FX_Orbitone, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Orbitone, MixerOutput::MainOutput, 0.5f);
    mixer->setReturnLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_PlateReverb, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, 0.5f);
    mixer->setReturnLevel(MixerOutput::FX_PlateReverb, MixerOutput::MainOutput, 0.5f);
    // DUMP2(mixer, cout, "Post setSendLevel & setReturnLevel");


    delete mixer;

    return 0;
}