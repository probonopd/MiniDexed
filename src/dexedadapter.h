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

#define DEXED_OP_ENABLE (DEXED_OP_OSC_DETUNE + 1)

// Some Dexed methods require to be guarded from being interrupted
// by other Dexed calls. This is done herein.

class CDexedAdapter : public Dexed
{
private:
	CSpinLock m_SpinLock;
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

	// Unison wrapper: for now, simulate unison by calling keydown/keyup multiple times with detune/pan offsets
	void keydown_unison(int16_t pitch, uint8_t velo, unsigned unisonVoices, unsigned unisonDetune, unsigned unisonSpread, int baseDetune, unsigned basePan) {
		if (unisonVoices < 1) unisonVoices = 1;
		for (unsigned v = 0; v < unisonVoices; ++v) {
			float detuneOffset = ((float)v - (unisonVoices - 1) / 2.0f) * (float)unisonDetune;
			int detune = baseDetune + (int)detuneOffset;
			m_SpinLock.Acquire();
			this->setMasterTune((int8_t)detune);
			Dexed::keydown(pitch, velo);
			m_SpinLock.Release();
		}
	}
	void keyup_unison(int16_t pitch, unsigned unisonVoices, unsigned unisonDetune, unsigned unisonSpread, int baseDetune, unsigned basePan) {
		if (unisonVoices < 1) unisonVoices = 1;
		for (unsigned v = 0; v < unisonVoices; ++v) {
			float detuneOffset = ((float)v - (unisonVoices - 1) / 2.0f) * (float)unisonDetune;
			int detune = baseDetune + (int)detuneOffset;
			m_SpinLock.Acquire();
			this->setMasterTune((int8_t)detune);
			Dexed::keyup(pitch);
			m_SpinLock.Release();
		}
	}

#ifdef ARM_ALLOW_MULTI_CORE
	// Stereo version for unison pan
	void getSamplesStereo(float32_t* bufferL, float32_t* bufferR, uint16_t n_samples) {
		m_SpinLock.Acquire();
		Dexed::getSamplesStereo(bufferL, bufferR, n_samples);
		m_SpinLock.Release();
	}
	// Set unison parameters for Dexed, now with base pan
	void setUnisonParameters(unsigned voices, unsigned detune, unsigned spread, float basePan) {
		m_SpinLock.Acquire();
		// Map detune: 0..99 -> 0..0.02 (2 cents)
		float detuneCents = ((float)detune / 99.0f) * 0.02f;
		Dexed::setUnisonParameters((uint8_t)voices, detuneCents, (float)spread / 99.0f, basePan);
		m_SpinLock.Release();
	}
#endif
};

#endif // _dexedadapter_h