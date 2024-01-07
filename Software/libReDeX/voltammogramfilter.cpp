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

#include "voltammogramfilter.h"

#include <algorithm>

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr size_t FilterSize = 121;
    constexpr size_t OffsetSize = (FilterSize - 1) / 2;
}

// ---------------------------------------------------------------------------------------------- //

void VoltammogramFilter::apply(std::span<const double> input, std::span<double> output)
{
    if (input.size() != output.size())
        throw Error("Filter output size must match input size.");

    m_data.resize(input.size() + FilterSize - 1);
    m_filter.apply(input, m_data);

    std::copy_n(std::begin(m_data) + OffsetSize, output.size(),  std::begin(output));
}

// ---------------------------------------------------------------------------------------------- //

auto VoltammogramFilter::makeFilter() -> SincFilter
{
    constexpr size_t SampleRate = 1000;

    constexpr double StopBand1Low  = 45.0 / SampleRate;
    constexpr double StopBand1High = 55.0 / SampleRate;

    constexpr double StopBand2Low  =  95.0 / SampleRate;
    constexpr double StopBand2High = 105.0 / SampleRate;

    constexpr double StopBand3Low  = 145.0 / SampleRate;
    constexpr double StopBand3High = 155.0 / SampleRate;

    constexpr double LowPassCutoff = 180.0 / SampleRate;

    constexpr auto StopBandWindow = SincFilter::WindowType::None;
    constexpr auto LowPassWindow  = SincFilter::WindowType::None;

    auto stopBand  = SincFilter::stopBand(FilterSize, StopBand1Low, StopBand1High, StopBandWindow);
    auto bandPass1 = SincFilter::bandPass(FilterSize, StopBand2Low, StopBand2High, StopBandWindow);
    auto bandPass2 = SincFilter::bandPass(FilterSize, StopBand3Low, StopBand3High, StopBandWindow);
    auto highPass  = SincFilter::highPass(FilterSize, LowPassCutoff, LowPassWindow);

    return stopBand - bandPass1 - bandPass2 - highPass;
}

// ---------------------------------------------------------------------------------------------- //
