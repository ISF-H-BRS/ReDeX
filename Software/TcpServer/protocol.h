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

#pragma once

#include <QString>

#include <initializer_list>

class Protocol
{
public:
    static constexpr const char* LineBreak = "\r\n";
    static constexpr size_t LineBreakLength = 2;

    static constexpr char TokenSeparator = 0x1f; // ASCII unit separator
    static constexpr char ValueSeparator = ';';

public:
    static auto joinTokens(const std::initializer_list<QString>& tokens) -> QString;

    template <typename... T>
    static auto joinTokens(T&&... args) {
        return joinTokens({ std::forward<T>(args)... });
    }

    static auto joinValues(const std::initializer_list<QString>& values) -> QString;

    template <typename... T>
    static auto joinValues(T&&... args) {
        return joinValues({ std::forward<T>(args)... });
    }
};
