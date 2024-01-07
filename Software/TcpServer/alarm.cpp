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

#include "alarm.h"
#include "assertions.h"

// ---------------------------------------------------------------------------------------------- //

auto Alarm::toString(Type type) -> QString
{
    switch (type)
    {
    case Type::Overvoltage:
        return "OVERVOLTAGE";

    case Type::Undervoltage:
        return "UNDERVOLTAGE";

    case Type::Overcurrent:
        return "OVERCURRENT";

    case Type::Overheat:
        return "OVERHEAT";

    default:
        FAIL();
        return "UNKNOWN";
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Alarm::toString(Severity severity) -> QString
{
    switch (severity)
    {
    case Severity::None:
        return "NONE";

    case Severity::Warning:
        return "WARNING";

    case Severity::Critical:
        return "CRITICAL";

    default:
        FAIL();
        return "UNKNOWN";
    }
}

// ---------------------------------------------------------------------------------------------- //
