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
// extra_features.h
//
// Header file that centralizes MACROS to enable / disable extra features
// Author: Vincent Gauché
//
#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

#if defined(ARM_ALLOW_MULTI_CORE)

    #if RASPPI < 3
        #define PLATE_REVERB_ENABLE   // Add support for the PlateReverb
    #else
        #define MIXING_CONSOLE_ENABLE // Add support for the MixingConsole
    #endif

#endif

#ifdef DEBUG

#include <iostream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <string>

using namespace std;

inline long long int getElapseTime(std::string marker = "")
{
    static std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> marker_times;

    auto current_time = std::chrono::high_resolution_clock::now();
    auto it = marker_times.find(marker);
    if (it != marker_times.end())
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - it->second);
        marker_times.erase(it);
        return duration.count();
    }
    else
    {
        marker_times[marker] = current_time;
        return 0;
    }
}

#define LAP_TIME(marker) getElapseTime(marker)
#define LOG_LAP_TIME(marker) { auto __d = getElapseTime(marker); if(__d > 0) std::cout << "Execution time for " << marker << ": " << __d << std::endl; }

#define DEBUG_VALUE(lbl, idx, v) std::cout << lbl << " " << idx << ": " << v << std::endl

#else

#define LAP_TIME(marker)
#define LOG_LAP_TIME(marker)

#endif
