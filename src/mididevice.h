//
// mididevice.h
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
#ifndef _mididevice_h
#define _mididevice_h

#include "config.h"
#include <circle/types.h>

class CMiniDexed;

class CMIDIDevice
{
public:
	enum TChannel
	{
		Channels = 16,
		OmniMode = Channels,
		Disabled,
		ChannelUnknown
	};

public:
	CMIDIDevice (CMiniDexed *pSynthesizer, CConfig *pConfig);
	~CMIDIDevice (void);

	void SetChannel (u8 ucChannel, unsigned nTG);
	u8 GetChannel (unsigned nTG) const;

protected:
	void MIDIMessageHandler (const u8 *pMessage, size_t nLength, unsigned nCable = 0);

private:
	CMiniDexed *m_pSynthesizer;
	CConfig *m_pConfig;

	u8 m_ChannelMap[CConfig::ToneGenerators];
};

#endif
