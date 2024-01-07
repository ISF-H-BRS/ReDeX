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

#include "wavegenerator.h"

#include <array>
#include <cmath>

// ---------------------------------------------------------------------------------------------- //

template class WaveGenerator<float>;
template class WaveGenerator<double>;

// ---------------------------------------------------------------------------------------------- //

template <typename T>
auto WaveGenerator<T>::sine(T time, T frequency, T amplitude, T offset) -> T
{
    T signal = std::sin(TwoPi * frequency * time);
    return amplitude * signal + offset;
}

// ---------------------------------------------------------------------------------------------- //

template <typename T>
auto WaveGenerator<T>::qsine(T time, T frequency, T amplitude, T offset) -> T
{
    static constexpr size_t LutSize = 1000;

    static std::array<T, LutSize> lut;
    static bool initialized = false;

    if (!initialized)
    {
        for (size_t i = 0; i < LutSize; ++i)
            lut[i] = std::sin(i * TwoPi / LutSize);

        initialized = true;
    }

    T period = frequency * time;
    period -= std::floor(period);

    if (period < Zero)
        period += One;

    T signal = lut[static_cast<size_t>(period * LutSize)];
    return amplitude * signal + offset;
}

// ---------------------------------------------------------------------------------------------- //

template <typename T>
auto WaveGenerator<T>::square(T time, T frequency, T amplitude, T offset) -> T
{
    static constexpr auto Threshold = static_cast<T>(0.5);

    T period = frequency * time;
    period -= std::floor(period);

    T signal = (period <= Threshold) ? One : -One;
    return amplitude * signal + offset;
}

// ---------------------------------------------------------------------------------------------- //

template <typename T>
auto WaveGenerator<T>::triangle(T time, T frequency, T amplitude, T offset) -> T
{
    static constexpr auto Threshold = static_cast<T>(0.5);
    static constexpr auto Multiplier = static_cast<T>(4.0);
    static constexpr auto Offset = static_cast<T>(0.25);

    T period = frequency * time;
    period += Offset;
    period -= std::floor(period);

    T signal = (period <= Threshold) ? (-One + Multiplier*period)
                                     : (+One - Multiplier*(period - Threshold));
    return amplitude * signal + offset;
}

// ---------------------------------------------------------------------------------------------- //

template <typename T>
auto WaveGenerator<T>::sawtooth(T time, T frequency, T amplitude, T offset) -> T
{
    static constexpr auto Multiplier = static_cast<T>(2.0);
    static constexpr auto Offset = static_cast<T>(0.5);

    T period = frequency * time;
    period += Offset;
    period -= std::floor(period);

    T signal = -One + Multiplier*period;
    return amplitude * signal + offset;
}

// ---------------------------------------------------------------------------------------------- //
