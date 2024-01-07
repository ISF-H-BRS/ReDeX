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

#include "protocol.h"

// ---------------------------------------------------------------------------------------------- //

auto Protocol::joinTokens(const std::initializer_list<QString>& tokens) -> QString
{
    QString result;

    for (const auto& token : tokens)
    {
        if (!result.isEmpty())
            result += Protocol::TokenSeparator;

        result += token;
    }

    return result;
}

// ---------------------------------------------------------------------------------------------- //

auto Protocol::joinValues(const std::initializer_list<QString>& values) -> QString
{
    QString result;

    for (const auto& value : values)
    {
        if (!result.isEmpty())
            result += Protocol::ValueSeparator;

        result += value;
    }

    return result;
}

// ---------------------------------------------------------------------------------------------- //
