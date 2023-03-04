#include "test_fx_helper.h"

#include <filesystem>

std::string getScenarioName(int scenario)
{
    std::stringstream ss;

    bool fxTube = Active(scenario, FXSwitch::FX__Tube);
    bool fxChorus = Active(scenario, FXSwitch::FX__Chorus);
    bool fxPhaser = Active(scenario, FXSwitch::FX__Phaser);
    bool fxOrbitone = Active(scenario, FXSwitch::FX__Orbitone);
    bool fxFlanger = Active(scenario, FXSwitch::FX__Flanger);
    bool fxDelay = Active(scenario, FXSwitch::FX__Delay);
    bool fxReverberator = Active(scenario, FXSwitch::FX__Reverberator);
    bool fxReverb = Active(scenario, FXSwitch::FX__PlateReverb);
    bool first = true;

    ss << "[ ";

    if(fxTube) 
    {
        // if(!first) ss << ", ";
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

    if(fxReverberator) 
    {
        if(!first) ss << ", ";
        ss << "Shim";
        // first = false;
    }

    ss << " ]";

    return ss.str();
}


void setupOuputStreamForCSV(std::ostream& out)
{
    struct comma_separator : std::numpunct<char>
    {
        virtual char do_decimal_point() const override { return ','; }
    };

    out.imbue(std::locale(out.getloc(), new comma_separator));
    out << std::fixed << std::showpoint;
}

bool createFolderStructure(std::string& path)
{
    try
    {
        std::filesystem::path file_path(path);
        if(!std::filesystem::exists(file_path.parent_path()))
        {
            std::filesystem::create_directories(file_path.parent_path());
        }

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

std::string getResultFile(const std::string& filename, bool createPath)
{
    std::string f = std::string(OUTPUT_FOLDER) + "/" + filename;
    if(createPath)
    {
        createFolderStructure(f);
    }

    return f;
}

float32_t getRandomValue()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float32_t> dist(-1.0f, 1.0f);

    return dist(gen);
}

float32_t** loadAudioTest(size_t& size, WaveHeader* hdr)
{
    float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size, hdr);
    assert(samples != nullptr);

    return samples;
}

void freeAudioSamples(float32_t** samples, size_t size)
{
    assert(samples != nullptr);

    for(size_t i = 0; i < size; ++i)
    {
        delete[] samples[i];
    }
    delete[] samples;
}
