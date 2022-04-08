//
// performanceconfig.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// Original author of this class:
//	R. Stange <rsta2@o2online.de>
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
#ifndef _performanceconfig_h
#define _performanceconfig_h

#include "config.h"
#include <fatfs/ff.h>
#include <Properties/propertiesfatfsfile.h>

class CPerformanceConfig	// Performance configuration
{
public:
	CPerformanceConfig (FATFS *pFileSystem);
	~CPerformanceConfig (void);

	bool Load (void);

	bool Save (void);

	// TG#
	unsigned GetBankNumber (unsigned nTG) const;		// 0 .. 127
	unsigned GetVoiceNumber (unsigned nTG) const;		// 0 .. 31
	unsigned GetMIDIChannel (unsigned nTG) const;		// 0 .. 15, omni, off
	unsigned GetVolume (unsigned nTG) const;		// 0 .. 127
	unsigned GetPan (unsigned nTG) const;			// 0 .. 127
	int GetDetune (unsigned nTG) const;			// -99 .. 99
	unsigned GetNoteLimitLow (unsigned nTG) const;		// 0 .. 127
	unsigned GetNoteLimitHigh (unsigned nTG) const;		// 0 .. 127
	int GetNoteShift (unsigned nTG) const;			// -24 .. 24

	void SetBankNumber (unsigned nValue, unsigned nTG);
	void SetVoiceNumber (unsigned nValue, unsigned nTG);
	void SetMIDIChannel (unsigned nValue, unsigned nTG);
	void SetVolume (unsigned nValue, unsigned nTG);
	void SetPan (unsigned nValue, unsigned nTG);
	void SetDetune (int nValue, unsigned nTG);
	void SetNoteLimitLow (unsigned nValue, unsigned nTG);
	void SetNoteLimitHigh (unsigned nValue, unsigned nTG);
	void SetNoteShift (int nValue, unsigned nTG);

	// Effects
	bool GetCompressorEnable (void) const;
	bool GetReverbEnable (void) const;
	unsigned GetReverbSize (void) const;			// 0 .. 99
	unsigned GetReverbHighDamp (void) const;		// 0 .. 99
	unsigned GetReverbLowDamp (void) const;			// 0 .. 99
	unsigned GetReverbLowPass (void) const;			// 0 .. 99
	unsigned GetReverbDiffusion (void) const;		// 0 .. 99
	unsigned GetReverbLevel (void) const;			// 0 .. 99

	void SetCompressorEnable (bool bValue);
	void SetReverbEnable (bool bValue);
	void SetReverbSize (unsigned nValue);
	void SetReverbHighDamp (unsigned nValue);
	void SetReverbLowDamp (unsigned nValue);
	void SetReverbLowPass (unsigned nValue);
	void SetReverbDiffusion (unsigned nValue);
	void SetReverbLevel (unsigned nValue);

private:
	CPropertiesFatFsFile m_Properties;

	unsigned m_nBankNumber[CConfig::ToneGenerators];
	unsigned m_nVoiceNumber[CConfig::ToneGenerators];
	unsigned m_nMIDIChannel[CConfig::ToneGenerators];
	unsigned m_nVolume[CConfig::ToneGenerators];
	unsigned m_nPan[CConfig::ToneGenerators];
	int m_nDetune[CConfig::ToneGenerators];
	unsigned m_nNoteLimitLow[CConfig::ToneGenerators];
	unsigned m_nNoteLimitHigh[CConfig::ToneGenerators];
	int m_nNoteShift[CConfig::ToneGenerators];

	bool m_bCompressorEnable;
	bool m_bReverbEnable;
	unsigned m_nReverbSize;
	unsigned m_nReverbHighDamp;
	unsigned m_nReverbLowDamp;
	unsigned m_nReverbLowPass;
	unsigned m_nReverbDiffusion;
	unsigned m_nReverbLevel;
};

#endif
