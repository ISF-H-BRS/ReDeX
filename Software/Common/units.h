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

#include <cmath>
#include <limits>
#include <tuple>

class Units
{
public:
    static auto adjust(double value, QString unit) -> std::tuple<double,QString>
    {
        const double absolute = std::abs(value);

        if (absolute == std::numeric_limits<double>::infinity())
            unit.clear();
        else if (absolute < 1.0)
        {
            if (absolute < 1.0e-12)
                value = 0.0;
            else if (absolute < 1.0e-9)
            {
                value *= 1.0e12;
                unit.prepend('p');
            }
            else if (absolute < 1.0e-6)
            {
                value *= 1.0e9;
                unit.prepend('n');
            }
            else if (absolute < 1.0e-3)
            {
                value *= 1.0e6;
                unit.prepend("µ");
            }
            else if (absolute < 1.0e-0)
            {
                value *= 1.0e3;
                unit.prepend('m');
            }
        }
        else
        {
            if (absolute > 1.0e12)
            {
                value /= 1.0e12;
                unit.prepend('T');
            }
            else if (absolute > 1.0e9)
            {
                value /= 1.0e9;
                unit.prepend('G');
            }
            else if (absolute > 1.0e6)
            {
                value /= 1.0e6;
                unit.prepend('M');
            }
            else if (absolute > 1.0e3)
            {
                value /= 1.0e3;
                unit.prepend("k");
            }
        }

        return { value, unit };
    }

    static auto roundValue(double value, unsigned int precision) -> double
    {
        auto digits = std::floor(std::log(std::abs(value)) / std::log(10.0)) + 1.0;
        auto shift = static_cast<double>(precision) - digits;
        auto pow10 = std::pow(10.0, shift);

        return std::round(value * pow10) / pow10;
    }

    static auto makeText(double value, unsigned int precision, const QString& unit) -> QString
    {
        const auto number = QString::number(roundValue(value, precision), 'f');
        return number.left(static_cast<int>(precision + 1)) + " " + unit;
    }

    static auto makeAdjustedText(double value, unsigned int precision,
                                 const QString& unit) -> QString
    {
        auto [newValue, newUnit] = adjust(value, unit);
        return makeText(newValue, precision, newUnit);
    }
};
