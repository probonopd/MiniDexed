#include "test_fx_helper.h"

std::string getScenarioName(int scenario)
{
    std::stringstream ss;

    bool fxTube = Active(scenario, FXSwitch::FX__Tube);
    bool fxChorus = Active(scenario, FXSwitch::FX__Chorus);
    bool fxPhaser = Active(scenario, FXSwitch::FX__Phaser);
    bool fxOrbitone = Active(scenario, FXSwitch::FX__Orbitone);
    bool fxFlanger = Active(scenario, FXSwitch::FX__Flanger);
    bool fxDelay = Active(scenario, FXSwitch::FX__Delay);
    bool fxShimmer = Active(scenario, FXSwitch::FX__ShimmerReverb);
    bool fxReverb = Active(scenario, FXSwitch::FX__PlateReverb);
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
        ss << "Chrs";
        first = false;
    }

    if(fxPhaser) 
    {
        if(!first) ss << ", ";
        ss << "Phsr";
        first = false;
    }

    if(fxOrbitone) 
    {
        if(!first) ss << ", ";
        ss << "Orbt";
        first = false;
    }

    if(fxFlanger) 
    {
        if(!first) ss << ", ";
        ss << "Flgr";
        first = false;
    }

    if(fxDelay) 
    {
        if(!first) ss << ", ";
        ss << "Dely";
        first = false;
    }

    if(fxReverb) 
    {
        if(!first) ss << ", ";
        ss << "Revb";
        first = false;
    }

    if(fxShimmer) 
    {
        if(!first) ss << ", ";
        ss << "Shim";
        first = false;
    }

    ss << " ]";

    return ss.str();
}


void setupOuputStreamFocCSV(std::ostream& out)
{
    struct comma_separator : numpunct<char>
    {
        virtual char do_decimal_point() const override { return ','; }
    };

    out.imbue(locale(out.getloc(), new comma_separator));
    out << fixed << showpoint;
}

std::string getResultFile(const std::string& filename)
{
    return std::string(OUTPUT_FOLDER) + "/" + filename;
}

float32_t getRandomValue()
{
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<float32_t> dist(-1.0f, 1.0f);

    return dist(gen);
}
