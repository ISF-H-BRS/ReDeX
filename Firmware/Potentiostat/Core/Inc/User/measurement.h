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

#include <chrono>
using namespace std::chrono_literals;

namespace Measurement {

    enum class Type
    {
        OpenCircuit,
        Electrolysis,
        LinearSweep,
        CyclicVoltammetry
    };

    constexpr size_t TypeCount = 4;

    template <typename T>
    constexpr auto toType(T index) { return static_cast<Type>(index); }

    enum class Gain
    {
        _20M,
        _4M,
        _40k,
        _400
    };

    constexpr size_t GainCount = 4;

    template <typename T>
    constexpr auto toGain(T index) { return static_cast<Gain>(index); }

    template <typename T = int>
    constexpr auto indexOf(Gain gain) { return static_cast<T>(gain); }

    constexpr auto DefaultGain = Gain::_400;

    constexpr std::chrono::seconds MinimumDuration = 1s;
    constexpr std::chrono::seconds MaximumDuration = 600s;

    constexpr int MinimumScanRate = 1;
    constexpr int MaximumScanRate = 100;

    constexpr int MinimumPotential = -2000;
    constexpr int MaximumPotential = +2000;

    constexpr int MinimumCycleCount = 1;
    constexpr int MaximumCycleCount = 10;

    struct Setup
    {
        Type type = Type::OpenCircuit;
        Gain gain = DefaultGain;
        std::chrono::seconds duration = MinimumDuration;
        int scanRate = MaximumScanRate;
        int vertex0 = 0;
        int vertex1 = 0;
        int vertex2 = 0;
        int cycleCount = MinimumCycleCount;
    };

    struct Sample
    {
        int gain;
        int voltage;
        int current;
    };
}
