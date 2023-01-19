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
//
#pragma once

#if defined(ARM_ALLOW_MULTI_CORE)

#define MIXING_CONSOLE_ENABLE //Add support for the MixingConsole

#endif

#ifdef DEBUG
#include <iostream>
#include <iomanip>
#endif