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

template <typename T>
class WaveGenerator
{
public:
    static constexpr auto One = static_cast<T>(1);
    static constexpr auto Zero = static_cast<T>(0);
    static constexpr auto TwoPi = static_cast<T>(2.0 * 3.14159265358979323846);

public:
    static auto sine(T time, T frequency, T amplitude = One, T offset = Zero) -> T;
    static auto qsine(T time, T frequency, T amplitude = One, T offset = Zero) -> T;
    static auto square(T time, T frequency, T amplitude = One, T offset = Zero) -> T;
    static auto triangle(T time, T frequency, T amplitude = One, T offset = Zero) -> T;
    static auto sawtooth(T time, T frequency, T amplitude = One, T offset = Zero) -> T;
};
