// ============================================================================================== //
//                                                                                                //
//  This file is part of the ISF ReDeX project.                                                   //
//                                                                                                //
//  Author:                                                                                       //
//  Marcel Hasler <mahasler@gmail.com>                                                            //
//                                                                                                //
//  Copyright (c) 2021 - 2023                                                                     //
//  Bonn-Rhein-Sieg University of Applied Sciences                                                //
//                                                                                                //
//  This program is free software: you can redistribute it and/or modify it under the terms       //
//  of the GNU General Public License as published by the Free Software Foundation, either        //
//  version 3 of the License, or (at your option) any later version.                              //
//                                                                                                //
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;     //
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     //
//  See the GNU General Public License for more details.                                          //
//                                                                                                //
//  You should have received a copy of the GNU General Public License along with this program.    //
//  If not, see <https://www.gnu.org/licenses/>.                                                  //
//                                                                                                //
// ============================================================================================== //

#include "errorstring.h"

#if defined(__linux__)
#include <cstring>
#elif defined(_WIN32)
#include <windows.h>
#endif

// ---------------------------------------------------------------------------------------------- //

auto ErrorString::fromCode(int code) -> std::string
{
#if defined(__linux__)
    return std::string(::strerror(code)) + ".";
#elif defined(_WIN32)
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
    LPTSTR buffer = nullptr;

    ::FormatMessage(flags, nullptr, code, 0, reinterpret_cast<LPTSTR>(&buffer), 0, nullptr);

    std::string string(buffer);
    ::LocalFree(buffer);

    return string;
#endif
}

// ---------------------------------------------------------------------------------------------- //

auto ErrorString::getLast() -> std::string
{
#if defined(__linux__)
    return fromCode(errno);
#elif defined(_WIN32)
    return fromCode(::GetLastError());
#endif
}

// ---------------------------------------------------------------------------------------------- //
