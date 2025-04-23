//
// udpmididevice.h
//
// Virtual midi device for data recieved on network 
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
#ifndef _udpmididevice_h
#define _udpmididevice_h

#include "mididevice.h"
#include "config.h"
#include "net/applemidi.h"
#include "net/udpmidi.h"

class CMiniDexed;

class CUDPMIDIDevice : CAppleMIDIHandler, CUDPMIDIHandler, public CMIDIDevice
{
public:
	CUDPMIDIDevice (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI);
	~CUDPMIDIDevice (void);

	boolean Initialize (void);
	virtual void OnAppleMIDIDataReceived(const u8* pData, size_t nSize) override;
	virtual void OnAppleMIDIConnect(const CIPAddress* pIPAddress, const char* pName) override;
	virtual void OnAppleMIDIDisconnect(const CIPAddress* pIPAddress, const char* pName) override;
	virtual void OnUDPMIDIDataReceived(const u8* pData, size_t nSize) override;

private:
	CMiniDexed *m_pSynthesizer;
	CConfig *m_pConfig;
	CBcmRandomNumberGenerator m_Random;
	CAppleMIDIParticipant* m_pAppleMIDIParticipant; // AppleMIDI participant instance
	CUDPMIDIReceiver* m_pUDPMIDIReceiver;
};

#endif
