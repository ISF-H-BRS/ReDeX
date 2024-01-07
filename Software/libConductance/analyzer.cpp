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

#include <conductance/analyzer.h>

#include <cassert>
#include <cmath>

// ---------------------------------------------------------------------------------------------- //

using namespace isf::Conductance;

// ---------------------------------------------------------------------------------------------- //

namespace {
    static constexpr double Pi = 3.14159265358979323846;
    static constexpr double TwoPi = 2.0 * Pi;
}

// ---------------------------------------------------------------------------------------------- //

Analyzer::Analyzer()
{
    m_sin.resize(Device::SampleRate);
    m_cos.resize(Device::SampleRate);

    setTargetFrequency(0);
}

// ---------------------------------------------------------------------------------------------- //

void Analyzer::setTargetFrequency(unsigned int frequency)
{
    static constexpr double SampleTime = 1.0 / Device::SampleRate;

    for (unsigned int i = 0; i < Device::SampleRate; ++i)
    {
        m_sin[i] = std::sin(TwoPi * frequency * i * SampleTime);
        m_cos[i] = std::cos(TwoPi * frequency * i * SampleTime);
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Analyzer::run(const DataBuffer& buffer) -> Magnitudes
{
    const double voltage = analyze(buffer, Device::Channel::Voltage);
    const double current = analyze(buffer, Device::Channel::Current);

    return { voltage, current };
}

// ---------------------------------------------------------------------------------------------- //

auto Analyzer::analyze(const DataBuffer& buffer, Device::Channel channel) -> double
{
    double x = 0.0;
    double y = 0.0;

    const auto& data = buffer.data(channel);

    for (unsigned int i = 0; i < Device::SampleRate; ++i)
    {
        x += m_cos[i] * data[i];
        y += m_sin[i] * data[i];
    }

    x /= Device::SampleRate/2;
    y /= Device::SampleRate/2;

    return std::sqrt(x*x + y*y);
}

// ---------------------------------------------------------------------------------------------- //

auto Analyzer::getValues(const Magnitudes& magnitudes,
                         Device::Gain gain, double leadResistance) -> std::tuple<double,double>
{
    const double current = magnitudes.current / Device::toDouble(gain);
    const double voltage = magnitudes.voltage - leadResistance * current;

    return { voltage, current };
}

// ---------------------------------------------------------------------------------------------- //
