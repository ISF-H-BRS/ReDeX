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

#include "defaultstring.h"
#include "enum.h"
#include "main.h"

#include <array>
#include <span>

class Sensor
{
public:
    enum class Type
    {
        Unknown,
        pH_ORP,
        Temperature,
        None
    };

    static constexpr size_t TypeCount = 4;
    static constexpr auto indexOf(Type type) { return Enum::indexOf(type); }

    static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

public:
    virtual ~Sensor() = default;

    virtual auto type() const -> Type = 0;
    virtual auto value() const -> double = 0;

    virtual void update() = 0;

    static auto toString(Type type) -> const char*;
};
