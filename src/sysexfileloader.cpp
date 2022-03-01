//
// sysexfileloader.cpp
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
#include "sysexfileloader.h"
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <circle/logger.h>

extern uint8_t voices_bank[1][32][156];

LOGMODULE ("syxfile");

uint8_t CSysExFileLoader::s_DefaultVoice[SizeSingleVoice] =	// FM-Piano
{
	95, 29, 20, 50, 99, 95, 00, 00, 41, 00, 19, 00, 00, 03, 00, 06, 79, 00, 01, 00, 14, // OP6 eg_rate_1-4, level_1-4, kbd_lev_scl_brk_pt, kbd_lev_scl_lft_depth, kbd_lev_scl_rht_depth, kbd_lev_scl_lft_curve, kbd_lev_scl_rht_curve, kbd_rate_scaling, amp_mod_sensitivity, key_vel_sensitivity, operator_output_level, osc_mode, osc_freq_coarse, osc_freq_fine, osc_detune
	95, 20, 20, 50, 99, 95, 00, 00, 00, 00, 00, 00, 00, 03, 00, 00, 99, 00, 01, 00, 00, // OP5
	95, 29, 20, 50, 99, 95, 00, 00, 00, 00, 00, 00, 00, 03, 00, 06, 89, 00, 01, 00, 07, // OP4
	95, 20, 20, 50, 99, 95, 00, 00, 00, 00, 00, 00, 00, 03, 00, 02, 99, 00, 01, 00, 07, // OP3
	95, 50, 35, 78, 99, 75, 00, 00, 00, 00, 00, 00, 00, 03, 00, 07, 58, 00, 14, 00, 07, // OP2
	96, 25, 25, 67, 99, 75, 00, 00, 00, 00, 00, 00, 00, 03, 00, 02, 99, 00, 01, 00, 10, // OP1
	94, 67, 95, 60, 50, 50, 50, 50,                                                     // 4 * pitch EG rates, 4 * pitch EG level
	04, 06, 00,                                                                         // algorithm, feedback, osc sync
	34, 33, 00, 00, 00, 04,                                                             // lfo speed, lfo delay, lfo pitch_mod_depth, lfo_amp_mod_depth, lfo_sync, lfo_waveform
	03, 24,                                                                             // pitch_mod_sensitivity, transpose
	70, 77, 45, 80, 73, 65, 78, 79, 00, 00                                              // 10 * char for name ("DEFAULT   ")
};

CSysExFileLoader::CSysExFileLoader (const char *pDirName)
:	m_DirName (pDirName),
	m_nBankID (0)
{
	m_DirName += "/voice";

	for (unsigned i = 0; i <= MaxVoiceBankID; i++)
	{
		m_pVoiceBank[i] = nullptr;
	}
}

CSysExFileLoader::~CSysExFileLoader (void)
{
	for (unsigned i = 0; i <= MaxVoiceBankID; i++)
	{
		delete m_pVoiceBank[i];
	}
}

void CSysExFileLoader::Load (void)
{
        DIR *pDirectory = opendir (m_DirName.c_str ());
	if (!pDirectory)
	{
		LOGWARN ("Directory %s not found", m_DirName.c_str ());

		return;
	}

	dirent *pEntry;
	while ((pEntry = readdir (pDirectory)) != nullptr)
	{
		unsigned nBank;
		size_t nLen = strlen (pEntry->d_name);

		if (   nLen < 5						// "[NNNN]N[_name].syx"
		    || strcmp (&pEntry->d_name[nLen-4], ".syx") != 0
		    || sscanf (pEntry->d_name, "%u", &nBank) != 1)
		{
			LOGWARN ("%s: Invalid filename format", pEntry->d_name);

			continue;
		}

		if (nBank > MaxVoiceBankID)
		{
			LOGWARN ("Bank #%u is not supported", nBank);

			continue;
		}

		if (m_pVoiceBank[nBank])
		{
			LOGWARN ("Bank #%u already loaded", nBank);

			continue;
		}

		m_pVoiceBank[nBank] = new TVoiceBank;
		assert (m_pVoiceBank[nBank]);

		std::string Filename (m_DirName);
		Filename += "/";
		Filename += pEntry->d_name;

		FILE *pFile = fopen (Filename.c_str (), "rb");
		if (pFile)
		{
			if (   fread (m_pVoiceBank[nBank], sizeof (TVoiceBank), 1, pFile) == 1
			    && m_pVoiceBank[nBank]->StatusStart == 0xF0
			    && m_pVoiceBank[nBank]->CompanyID   == 0x43
			    && m_pVoiceBank[nBank]->Format      == 0x09
			    && m_pVoiceBank[nBank]->StatusEnd   == 0xF7)
			{
				LOGDBG ("Bank #%u successfully loaded", nBank);
			}
			else
			{
				LOGWARN ("%s: Invalid size or format", Filename.c_str ());

				delete m_pVoiceBank[nBank];
				m_pVoiceBank[nBank] = nullptr;
			}

			fclose (pFile);
		}
		else
		{
			delete m_pVoiceBank[nBank];
			m_pVoiceBank[nBank] = nullptr;
		}
	}

	closedir (pDirectory);
}

void CSysExFileLoader::SelectVoiceBank (unsigned nBankID)
{
	if (nBankID <= MaxVoiceBankID)
	{
		m_nBankID = nBankID;
	}
}

void CSysExFileLoader::GetVoice (unsigned nVoiceID, uint8_t *pVoiceData)
{
	if (nVoiceID <= VoicesPerBank)
	{
		assert (m_nBankID <= MaxVoiceBankID);
		if (m_pVoiceBank[m_nBankID])
		{
			DecodePackedVoice (m_pVoiceBank[m_nBankID]->Voice[nVoiceID], pVoiceData);

			return;
		}
		else
		{
			// Use default voices_bank instead of s_DefaultVoice for bank 0,
			// if the bank was not successfully loaded from disk.
			if (m_nBankID == 0)
			{
				memcpy (pVoiceData, voices_bank[0][nVoiceID], SizeSingleVoice);

				return;
			}
		}
	}

	memcpy (pVoiceData, s_DefaultVoice, SizeSingleVoice);
}

// See: https://github.com/bwhitman/learnfm/blob/master/dx7db.py
void CSysExFileLoader::DecodePackedVoice (const uint8_t *pPackedData, uint8_t *pDecodedData)
{
	const uint8_t *p = pPackedData;
	uint8_t *o = pDecodedData;

	memset (o, 0, 156);

	for (unsigned op = 0; op < 6; op++)
	{
		memcpy(&o[op*21], &p[op*17], 11);

		uint8_t leftrightcurves = p[op*17+11];
		o[op * 21 + 11] = leftrightcurves & 3;
		o[op * 21 + 12] = (leftrightcurves >> 2) & 3;

		uint8_t detune_rs = p[op * 17 + 12];
		o[op * 21 + 13] = detune_rs & 7;
		o[op * 21 + 20] = detune_rs >> 3;

		uint8_t kvs_ams = p[op * 17 + 13];
		o[op * 21 + 14] = kvs_ams & 3;
		o[op * 21 + 15] = kvs_ams >> 2;
		o[op * 21 + 16] = p[op * 17 + 14];

		uint8_t fcoarse_mode = p[op * 17 + 15];
		o[op * 21 + 17] = fcoarse_mode & 1;
		o[op * 21 + 18] = fcoarse_mode >> 1;
		o[op * 21 + 19] = p[op * 17 + 16];
	}

	memcpy (&o[126], &p[102], 9);
	uint8_t oks_fb = p[111];
	o[135] = oks_fb & 7;
	o[136] = oks_fb >> 3;
	memcpy (&o[137], &p[112], 4);
	uint8_t lpms_lfw_lks = p[116];
	o[141] = lpms_lfw_lks & 1;
	o[142] = (lpms_lfw_lks >> 1) & 7;
	o[143] = lpms_lfw_lks >> 4;
	memcpy (&o[144], &p[117], 11);
	o[155] = 0x3f;

	// Clamp the unpacked patches to a known max.
	static uint8_t maxes[] =
	{
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // osc6
		3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // osc5
		3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // osc4
		3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // osc3
		3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // osc2
		3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // osc1
		3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
		99, 99, 99, 99, 99, 99, 99, 99, // pitch eg rate & level
		31, 7, 1, 99, 99, 99, 99, 1, 5, 7, 48, // algorithm etc
		126, 126, 126, 126, 126, 126, 126, 126, 126, 126, // name
		127 // operator on/off
	};

	for (unsigned i = 0; i < 156; i++)
	{
		if (o[i] > maxes[i])
		{
			o[i] = maxes[i];
		}
	}
}
