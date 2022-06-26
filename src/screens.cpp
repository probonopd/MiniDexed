#include "screens.h"

CPerformanceScreen::CPerformanceScreen() : 
    CScreen(PerformanceScreen, "Performance", PerformanceScreenPageCount)
{
}

CPerformanceScreen::~CPerformanceScreen()
{
}

CPerformanceEditScreen::CPerformanceEditScreen() :
    CScreen(PerformanceEditScreen, "Perf. edit", PerformanceEditScreenPageCount)
{
}

CPerformanceEditScreen::~CPerformanceEditScreen()
{
}

CVoiceScreen::CVoiceScreen() : 
    CScreen(VoiceScreen, "Voice", VoiceScreenPageCount)
{
}

CVoiceScreen::~CVoiceScreen()
{
}

CVoiceEditScreen::CVoiceEditScreen() : 
    CScreen(VoiceEditScreen, "Voice edit", VoiceEditScreenPageCount)
{
}

CVoiceEditScreen::~CVoiceEditScreen()
{
}

COperatorScreen::COperatorScreen() : 
    CScreen(OperatorScreen, "Op. edit", OperatorScreenPageCount)
{
}

COperatorScreen::~COperatorScreen()
{
}

CFxScreen::CFxScreen() :
    CScreen(FxScreen, "FX", FxScreenPageCount)
{
}

CFxScreen::~CFxScreen()
{
}

CNameEditScreen::CNameEditScreen(const char* pszScreenName) :
    CScreen(NameEditScreen, pszScreenName, NameEditScreenPageCount)
{
}

CNameEditScreen::~CNameEditScreen()
{
}
