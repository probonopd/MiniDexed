//
// minidexed.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _screens_h
#define _screens_h

#include <ui.h>
#include <minidexed.h>

enum ScreenID
{
    PerformanceScreen = 0,
    PerformanceEditScreen,
    VoiceScreen,
    VoiceEditScreen,
    OperatorScreen,
    FxScreen,
    NameEditScreen
};

enum ScreenPageCount
{
    PerformanceScreenPageCount = 1,
    PerformanceEditScreenPageCount = 3,
    VoiceScreenPageCount = 1,
    VoiceEditScreenPageCount = 3,
    OperatorScreenPageCount = 1,
    FxScreenPageCount = 1,
    NameEditScreen = 1
};

class CPerformanceScreen : public CScreen 
{
public:
    CPerformanceScreen();
    virtual ~CPerformanceScreen();
};

class CPerformanceEditScreen : public CScreen 
{
public:
    CPerformanceEditScreen();
    virtual ~CPerformanceEditScreen();
};

class CVoiceScreen : public CScreen 
{
public:
    CVoiceScreen();
    virtual ~CVoiceScreen();
};

class CVoiceEditScreen : public CScreen 
{
public:
    CVoiceEditScreen();
    virtual ~CVoiceEditScreen();
};

class COperatorScreen : public CScreen 
{
public:
    COperatorScreen();
    virtual ~COperatorScreen();
};

class CFxScreen : public CScreen
{
public:
    CFxScreen();
    ~CFxScreen();
};

class CNameEditScreen : public CScreen
{
public:
    CNameEditScreen();
    ~CNameEditScreen();
};

#endif