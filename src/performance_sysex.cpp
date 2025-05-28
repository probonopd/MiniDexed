//
// performance_sysex_handler.cpp
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2025  The MiniDexed Team
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

#include "performance_sysex.h"
#include "performanceconfig.h"
#include <circle/logger.h>
#include <cstring>
#include "minidexed.h"
#include "mididevice.h"

// Helper functions for standard MIDI 14-bit signed value encoding/decoding (offset method)
static void encode_midi14_signed_7bit(int value, uint8_t& msb, uint8_t& lsb) {
    // Clamp to 14-bit signed range
    if (value < -8192) value = -8192;
    if (value > 8191) value = 8191;
    uint16_t midi14 = (uint16_t)(value + 8192); // MIDI standard: center is 8192
    msb = (midi14 >> 7) & 0x7F;
    lsb = midi14 & 0x7F;
}
static int decode_midi14_signed_7bit(uint8_t msb, uint8_t lsb) {
    uint16_t midi14 = ((msb & 0x7F) << 7) | (lsb & 0x7F);
    return (int)midi14 - 8192;
}

LOGMODULE ("PerformanceSysEx");

static constexpr uint8_t SYSEX_START = 0xF0;
static constexpr uint8_t SYSEX_END   = 0xF7;
static constexpr uint8_t MANUFACTURER_ID = 0x7D;
static constexpr uint8_t GET_GLOBAL = 0x10;
static constexpr uint8_t GET_TG     = 0x11;
static constexpr uint8_t SET_GLOBAL = 0x20;
static constexpr uint8_t SET_TG     = 0x21;

static void send_sysex_response(const uint8_t* data, size_t len, CMIDIDevice* pDevice, unsigned nCable) {
    if (pDevice) {
        pDevice->Send(data, len, nCable);
        LOGNOTE("Sent SysEx response (%u bytes) on cable %u", (unsigned)len, nCable);
    }
}

void handle_performance_sysex(const uint8_t* pMessage, CMIDIDevice* pDevice, CMiniDexed* miniDexed, unsigned nCable) {
    if (!pMessage || !miniDexed) {
        LOGERR("handle_performance_sysex: Null pointer or MiniDexed not set");
        return;
    }
    CPerformanceConfig* perf = miniDexed->GetPerformanceConfig();
    if (!perf) {
        LOGERR("handle_performance_sysex: PerformanceConfig not set in MiniDexed");
        return;
    }
    if (pMessage[0] != SYSEX_START) {
        LOGERR("SysEx: Invalid start byte: 0x%02X", pMessage[0]);
        return;
    }
    int len = 0;
    while (len < 256 && pMessage[len] != SYSEX_END) ++len;
    if (len < 3 || pMessage[len] != SYSEX_END) {
        LOGERR("SysEx: No end byte or too short");
        return;
    }
    if (pMessage[1] != MANUFACTURER_ID) {
        LOGERR("SysEx: Invalid manufacturer ID: 0x%02X", pMessage[1]);
        return;
    }

    uint8_t tg = 0xFF; // 0xFF = global
    uint8_t cmd = 0;
    uint8_t offset = 0;

    // New protocol: 0x10 = global GET, 0x11 = per-TG GET, 0x20 = global SET, 0x21 = per-TG SET
    if (pMessage[2] == GET_GLOBAL) {
        cmd = GET_GLOBAL;
        offset = 3;
        LOGNOTE("SysEx: Global GET request");
    } else if (pMessage[2] == GET_TG) {
        cmd = GET_TG;
        // Allow both F0 7D 11 <TG> F7 (all params) and F0 7D 11 <TG> <param> <param> F7 (single param)
        if (len < 4) {
            LOGERR("SysEx: Per-TG GET request too short");
            return;
        }
        tg = pMessage[3];
        offset = 4;
        LOGNOTE("SysEx: TG-specific GET request for TG %u", tg);
    } else if (pMessage[2] == SET_GLOBAL) {
        cmd = SET_GLOBAL;
        offset = 3;
        LOGNOTE("SysEx: Global SET request");
    } else if (pMessage[2] == SET_TG) {
        cmd = SET_TG;
        if (len < 5) {
            LOGERR("SysEx: Per-TG SET request too short");
            return;
        }
        tg = pMessage[3];
        offset = 4;
        LOGNOTE("SysEx: TG-specific SET request for TG %u", tg);
    } else {
        LOGERR("SysEx: Unrecognized message structure");
        return;
    }

    if (offset == len) {
        // Dump all global or all TG parameters
        if (cmd == GET_GLOBAL) {
            // Dump all global
            size_t count = 0;
            const uint16_t* params = CPerformanceConfig::GetAllGlobalParams(count);
            uint16_t values[16] = {0};
            if (perf->GetGlobalParameters(params, values, count)) {
                LOGNOTE("SysEx: Dumping all global parameters");
                uint8_t resp[64] = {0};
                resp[0] = SYSEX_START;
                resp[1] = MANUFACTURER_ID;
                resp[2] = SET_GLOBAL; // F0 7D 20 ...
                size_t idx = 3;
                for (size_t i = 0; i < count; ++i) {
                    resp[idx++] = (params[i] >> 8) & 0xFF;
                    resp[idx++] = params[i] & 0xFF;
                    resp[idx++] = (values[i] >> 8) & 0xFF;
                    resp[idx++] = values[i] & 0xFF;
                    LOGNOTE("  Param 0x%04X = 0x%04X", params[i], values[i]);
                }
                resp[idx++] = SYSEX_END;
                send_sysex_response(resp, idx, pDevice, nCable);
            } else {
                LOGERR("SysEx: Failed to get all global parameters");
            }
        } else if (cmd == GET_TG) {
            // Dump all TG
            size_t count = 0;
            const uint16_t* params = CPerformanceConfig::GetAllTGParams(count);
            uint16_t values[32] = {0};
            if (perf->GetTGParameters(params, values, count, tg)) {
                LOGNOTE("SysEx: Dumping all TG parameters for TG %u", tg);
                uint8_t resp[128] = {0};
                resp[0] = SYSEX_START;
                resp[1] = MANUFACTURER_ID;
                resp[2] = SET_TG; // F0 7D 21 nn ...
                resp[3] = tg;
                size_t idx = 4;
                for (size_t i = 0; i < count; ++i) {
                    resp[idx++] = (params[i] >> 8) & 0xFF;
                    resp[idx++] = params[i] & 0xFF;
                    resp[idx++] = (values[i] >> 8) & 0xFF;
                    resp[idx++] = values[i] & 0xFF;
                    LOGNOTE("  Param 0x%04X = 0x%04X", params[i], values[i]);
                }
                resp[idx++] = SYSEX_END;
                send_sysex_response(resp, idx, pDevice, nCable);
            } else {
                LOGERR("SysEx: Failed to get all TG parameters for TG %u", tg);
            }
        }
        return;
    }
    while (offset + 1 < len) {
        if (pMessage[offset] == SYSEX_END) break;
        uint16_t param = (pMessage[offset] << 8) | pMessage[offset+1];
        offset += 2;
        if (cmd == GET_GLOBAL) {
            uint16_t value = 0;
            bool ok = false;
            ok = perf->GetGlobalParameters(&param, &value, 1);
            if (param == CPerformanceConfig::PARAM_DETUNE || param == CPerformanceConfig::PARAM_NOTE_SHIFT) {
                uint8_t msb, lsb;
                encode_midi14_signed_7bit((int16_t)value, msb, lsb);
                uint8_t resp[9] = {SYSEX_START, MANUFACTURER_ID, SET_GLOBAL, (uint8_t)(param >> 8), (uint8_t)(param & 0xFF), msb, lsb, SYSEX_END};
                send_sysex_response(resp, 8, pDevice, nCable);
                continue;
            }
            LOGNOTE("SysEx: GET global param 0x%04X -> 0x%04X (%s)", param, value, ok ? "OK" : "FAIL");
            // Build and send response (use SET_GLOBAL as response type)
            uint8_t resp[8] = {SYSEX_START, MANUFACTURER_ID, SET_GLOBAL, (uint8_t)(param >> 8), (uint8_t)(param & 0xFF), (uint8_t)(value >> 8), (uint8_t)(value & 0xFF), SYSEX_END};
            send_sysex_response(resp, 8, pDevice, nCable);
        } else if (cmd == GET_TG) {
            uint16_t value = 0;
            bool ok = false;
            ok = perf->GetTGParameters(&param, &value, 1, tg);
            if (param == CPerformanceConfig::PARAM_DETUNE || param == CPerformanceConfig::PARAM_NOTE_SHIFT) {
                uint8_t msb, lsb;
                encode_midi14_signed_7bit((int16_t)value, msb, lsb);
                uint8_t resp[11] = {SYSEX_START, MANUFACTURER_ID, SET_TG, tg, (uint8_t)(param >> 8), (uint8_t)(param & 0xFF), msb, lsb, SYSEX_END};
                send_sysex_response(resp, 9, pDevice, nCable);
                continue;
            }
            LOGNOTE("SysEx: GET TG %u param 0x%04X -> 0x%04X (%s)", tg, param, value, ok ? "OK" : "FAIL");
            uint8_t resp[10] = {SYSEX_START, MANUFACTURER_ID, SET_TG, tg, (uint8_t)(param >> 8), (uint8_t)(param & 0xFF), (uint8_t)(value >> 8), (uint8_t)(value & 0xFF), SYSEX_END};
            send_sysex_response(resp, 9, pDevice, nCable);
        } else if (cmd == SET_GLOBAL) {
            if (offset + 1 >= len) {
                LOGERR("SysEx: Global SET param 0x%04X missing value bytes", param);
                break;
            }
            uint16_t value = (pMessage[offset] << 8) | pMessage[offset+1];
            if (param == CPerformanceConfig::PARAM_DETUNE || param == CPerformanceConfig::PARAM_NOTE_SHIFT) {
                int decoded = decode_midi14_signed_7bit(pMessage[offset], pMessage[offset+1]);
                value = (uint16_t)decoded;
            }
            offset += 2;
            bool ok = perf->SetGlobalParameters(&param, &value, 1);
            LOGNOTE("SysEx: SET global param 0x%04X = 0x%04X (%s)", param, value, ok ? "OK" : "FAIL");
            if (ok && miniDexed) {
                miniDexed->LoadPerformanceParameters();
            }
        } else if (cmd == SET_TG) {
            if (offset + 1 >= len) {
                LOGERR("SysEx: TG SET param 0x%04X missing value bytes", param);
                break;
            }
            uint16_t value = (pMessage[offset] << 8) | pMessage[offset+1];
            if (param == CPerformanceConfig::PARAM_DETUNE || param == CPerformanceConfig::PARAM_NOTE_SHIFT) {
                int decoded = decode_midi14_signed_7bit(pMessage[offset], pMessage[offset+1]);
                value = (uint16_t)decoded;
            }
            offset += 2;
            bool ok = perf->SetTGParameters(&param, &value, 1, tg);
            LOGNOTE("SysEx: SET TG %u param 0x%04X = 0x%04X (%s)", tg, param, value, ok ? "OK" : "FAIL");
            if (ok && miniDexed) {
                miniDexed->LoadPerformanceParameters();
            }
        }
    }
}
