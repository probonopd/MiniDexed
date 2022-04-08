//
// dexedadapter.h
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

#ifndef _dexedadapter_h
#define _dexedadapter_h

#include <synth_dexed.h>
#include <circle/spinlock.h>
#include <stdint.h>

// Some Dexed methods require to be guarded from being interrupted
// by other Dexed calls. This is done herein.

class CDexedAdapter : public Dexed
{
public:
	CDexedAdapter (uint8_t maxnotes, int rate)
	: Dexed (maxnotes, rate)
	{
	}

	void loadVoiceParameters (uint8_t* data)
	{
		m_SpinLock.Acquire ();
		Dexed::loadVoiceParameters (data);
		m_SpinLock.Release ();
	}

	void keyup (int16_t pitch)
	{
		m_SpinLock.Acquire ();
		Dexed::keyup (pitch);
		m_SpinLock.Release ();
	}

	void keydown (int16_t pitch, uint8_t velo)
	{
		m_SpinLock.Acquire ();
		Dexed::keydown (pitch, velo);
		m_SpinLock.Release ();
	}

	void getSamples (float32_t* buffer, uint16_t n_samples)
	{
		m_SpinLock.Acquire ();
		Dexed::getSamples (buffer, n_samples);
		m_SpinLock.Release ();
	}

	void ControllersRefresh (void)
	{
		m_SpinLock.Acquire ();
		Dexed::ControllersRefresh ();
		m_SpinLock.Release ();
	}

	void setSustain (bool sustain)
	{
		m_SpinLock.Acquire ();
		Dexed::setSustain (sustain);
		m_SpinLock.Release ();
	}

private:
	CSpinLock m_SpinLock;
};

#endif
