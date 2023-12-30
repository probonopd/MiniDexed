//
// usbminidexedmidigadget.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// Based on circle/usb/gadget/usbmidigadget.h
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
#ifndef _usbminidexedmidigadget_h
#define _usbminidexedmidigadget_h

#include <circle/usb/gadget/usbmidigadget.h>
#include <circle/usb/gadget/usbmidigadgetendpoint.h>
#include <circle/sysconfig.h>
#include <assert.h>

class CUSBMiniDexedMIDIGadget : public CUSBMIDIGadget
{
private:
#define MDSTRINGDESCRIPTORS 3
	const char *const s_MiniDexedStringDescriptor[MDSTRINGDESCRIPTORS] =
	{
		"\x04\x03\x09\x04",		// Language ID
		"probonopd",
		"MiniDexed"
	};

public:
	CUSBMiniDexedMIDIGadget (CInterruptSystem *pInterruptSystem)
	: CUSBMIDIGadget (pInterruptSystem)
	{
	}

	~CUSBMiniDexedMIDIGadget (void)
	{
		assert(0);
	}

protected:
	// Override GetDescriptor frmo CUSBMIDIGadget.
	// See CUSBMIDIGadget for details.
	// This will only act on the DESCRIPOR_STRING.
	// All other descriptors are returned from USBMIDIGadget.
	//
	const void *GetDescriptor (u16 wValue, u16 wIndex, size_t *pLength) override
	{
		assert (pLength);

		u8 uchDescIndex = wValue & 0xFF;

		switch (wValue >> 8)
		{
		case DESCRIPTOR_STRING:
			if (!uchDescIndex)
			{
				*pLength = (u8) s_MiniDexedStringDescriptor[0][0];
				return s_MiniDexedStringDescriptor[0];
			}
			else if (uchDescIndex < MDSTRINGDESCRIPTORS)
			{
				return CUSBMIDIGadget::ToStringDescriptor (s_MiniDexedStringDescriptor[uchDescIndex], pLength);
			}
			break;

		default:
			break;
		}

		return CUSBMIDIGadget::GetDescriptor(wValue, wIndex, pLength);
	}
};

#endif
