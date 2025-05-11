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

LOGMODULE ("PerformanceSysEx");

static constexpr uint8_t SYSEX_START = 0xF0;
static constexpr uint8_t SYSEX_END   = 0xF7;
static constexpr uint8_t MANUFACTURER_ID = 0x7D;
static constexpr uint8_t CMD_GET = 0x10;
static constexpr uint8_t CMD_SET = 0x20;

static void send_sysex_response(const uint8_t* data, size_t len, CMIDIDevice* pDevice, unsigned nCable) {
    if (pDevice) {
        pDevice->Send(data, len, nCable);
        LOGNOTE("Sent SysEx response (%u bytes) on cable %u", (unsigned)len, nCable);
    }
}

void handle_performance_sysex(const uint8_t* pMessage, CMIDIDevice* pDevice, CPerformanceConfig* perf, unsigned nCable) {
    if (!pMessage || !perf) {
        LOGERR("handle_performance_sysex: Null pointer or PerformanceConfig not set");
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

    if (pMessage[2] == CMD_GET || pMessage[2] == CMD_SET) {
        cmd = pMessage[2];
        offset = 3;
        LOGNOTE("SysEx: Global %s request", cmd == CMD_GET ? "GET" : "SET");
    } else if (pMessage[2] < 0x80 && (pMessage[3] == CMD_GET || pMessage[3] == CMD_SET)) {
        tg = pMessage[2];
        cmd = pMessage[3];
        offset = 4;
        LOGNOTE("SysEx: TG-specific %s request for TG %u", cmd == CMD_GET ? "GET" : "SET", tg);
    } else {
        LOGERR("SysEx: Unrecognized message structure");
        return;
    }

    if (offset == len) {
        // Dump all global or all TG parameters
        if (cmd == CMD_GET) {
            if (tg == 0xFF) {
                // Dump all global
                size_t count = 0;
                const uint16_t* params = CPerformanceConfig::GetAllGlobalParams(count);
                uint16_t values[16] = {0};
                if (perf->GetGlobalParameters(params, values, count)) {
                    LOGNOTE("SysEx: Dumping all global parameters");
                    uint8_t resp[64] = {0};
                    resp[0] = SYSEX_START;
                    resp[1] = MANUFACTURER_ID;
                    resp[2] = CMD_GET;
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
            } else {
                // Dump all TG
                size_t count = 0;
                const uint16_t* params = CPerformanceConfig::GetAllTGParams(count);
                uint16_t values[32] = {0};
                if (perf->GetTGParameters(params, values, count, tg)) {
                    LOGNOTE("SysEx: Dumping all TG parameters for TG %u", tg);
                    uint8_t resp[128] = {0};
                    resp[0] = SYSEX_START;
                    resp[1] = MANUFACTURER_ID;
                    resp[2] = tg;
                    resp[3] = CMD_GET;
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
        }
        return;
    }
    while (offset + 1 < len) {
        uint16_t param = (pMessage[offset] << 8) | pMessage[offset+1];
        offset += 2;
        if (cmd == CMD_GET) {
            uint16_t value = 0;
            bool ok = false;
            if (tg == 0xFF) {
                ok = perf->GetGlobalParameters(&param, &value, 1);
                LOGNOTE("SysEx: GET global param 0x%04X -> 0x%04X (%s)", param, value, ok ? "OK" : "FAIL");
                // Build and send response
                uint8_t resp[9] = {SYSEX_START, MANUFACTURER_ID, CMD_GET, (uint8_t)(param >> 8), (uint8_t)(param & 0xFF), (uint8_t)(value >> 8), (uint8_t)(value & 0xFF), SYSEX_END};
                send_sysex_response(resp, 8, pDevice, nCable);
            } else {
                ok = perf->GetTGParameters(&param, &value, 1, tg);
                LOGNOTE("SysEx: GET TG %u param 0x%04X -> 0x%04X (%s)", tg, param, value, ok ? "OK" : "FAIL");
                uint8_t resp[10] = {SYSEX_START, MANUFACTURER_ID, tg, CMD_GET, (uint8_t)(param >> 8), (uint8_t)(param & 0xFF), (uint8_t)(value >> 8), (uint8_t)(value & 0xFF), SYSEX_END};
                send_sysex_response(resp, 9, pDevice, nCable);
            }
        } else if (cmd == CMD_SET) {
            if (offset + 1 >= len) {
                LOGERR("SysEx: SET param 0x%04X missing value bytes", param);
                break;
            }
            uint16_t value = (pMessage[offset] << 8) | pMessage[offset+1];
            offset += 2;
            bool ok = false;
            if (tg == 0xFF) {
                ok = perf->SetGlobalParameters(&param, &value, 1);
                LOGNOTE("SysEx: SET global param 0x%04X = 0x%04X (%s)", param, value, ok ? "OK" : "FAIL");
            } else {
                ok = perf->SetTGParameters(&param, &value, 1, tg);
                LOGNOTE("SysEx: SET TG %u param 0x%04X = 0x%04X (%s)", tg, param, value, ok ? "OK" : "FAIL");
            }
        }
    }
}

/*
Examples of MiniDexed Performance SysEx messages and expected handler behavior:

1. Get a Global Parameter (e.g., CompressorEnable)
  Send: F0 7D 10 00 00 F7
  - Logs GET for global param 0x0000, queries value, logs result.

2. Set a Global Parameter (e.g., ReverbEnable ON)
  Send: F0 7D 20 00 01 00 01 F7
  - Logs SET for global param 0x0001, updates value, logs result.

3. Get All Global Parameters
  Send: F0 7D 10 F7
  - Logs and dumps all global parameters and values.

4. Get a TG Parameter (e.g., Volume for TG 2)
  Send: F0 7D 02 10 00 03 F7
  - Logs GET for TG 2 param 0x0003, queries value, logs result.

5. Set a TG Parameter (e.g., Pan for TG 1 to 64)
  Send: F0 7D 01 20 00 04 00 40 F7
  - Logs SET for TG 1 param 0x0004, updates value, logs result.

6. Get All Parameters for TG 0
  Send: F0 7D 00 10 F7
  - Logs and dumps all TG 0 parameters and values.

7. Malformed or Unknown Parameter
  Send: F0 7D 10 12 34 F7
  - Logs GET for global param 0x1234, logs FAIL if unsupported.

*/
