//
// .h
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
#ifndef _rtpmididevice_h
#define _rtpmididevice_h

#include "mididevice.h"
#include "config.h"
#include "net/applemidi.h"

class CMiniDexed;

class CRTPMIDIDevice : CAppleMIDIHandler, CMIDIDevice
{
public:
	CRTPMIDIDevice (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI);
	~CRTPMIDIDevice (void);

	boolean Initialize (void);
	virtual void OnAppleMIDIDataReceived(const u8* pData, size_t nSize) override;
	virtual void OnAppleMIDIConnect(const CIPAddress* pIPAddress, const char* pName) override;
	virtual void OnAppleMIDIDisconnect(const CIPAddress* pIPAddress, const char* pName) override;

private:
	CConfig *m_pConfig;
    CBcmRandomNumberGenerator m_Random;
	CAppleMIDIParticipant* m_pAppleMIDIParticipant;
};

#endif
