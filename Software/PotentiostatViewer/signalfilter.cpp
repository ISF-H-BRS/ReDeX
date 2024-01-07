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

#include "signalfilter.h"

// ---------------------------------------------------------------------------------------------- //

void SignalFilter::reset()
{
    m_buffer.clear();
    m_pendingSamples = 0;
}

// ---------------------------------------------------------------------------------------------- //

void SignalFilter::update(InputBuffer input, OutputBuffer* output)
{
    if (input.empty())
        return;

    if (m_buffer.empty())
    {
        for (size_t i = 0; i < (m_filterSize - 1) / 2; ++i)
            m_buffer.push_front(input[0]);
    }

    for (size_t i = 0; i < input.size(); ++i)
    {
        ++m_pendingSamples;
        process(input[i], output);
    }
}

// ---------------------------------------------------------------------------------------------- //

void SignalFilter::finalize(OutputBuffer* output)
{
    if (m_buffer.empty())
        return;

    const double sample = m_buffer.back();

    while (m_pendingSamples > 0)
        process(sample, output);
}

// ---------------------------------------------------------------------------------------------- //

inline
void SignalFilter::process(double input, OutputBuffer* output)
{
    m_buffer.push_back(input);

    if (m_buffer.size() >= m_filterSize)
    {
        while (m_buffer.size() > m_filterSize)
            m_buffer.pop_front();

        const auto& coeffs = m_filter.coefficients();

        double value = 0.0;

        for (size_t i = 0; i < m_filterSize; ++i)
            value += coeffs[i] * m_buffer[i];

        output->push_back(value);
        --m_pendingSamples;
    }
}

// ---------------------------------------------------------------------------------------------- //

auto SignalFilter::makeFilter() -> SincFilter
{
    constexpr size_t FilterSize = 121;

    constexpr double StopBand1Low  = 45.0 / Device::SampleRate;
    constexpr double StopBand1High = 55.0 / Device::SampleRate;

    constexpr double StopBand2Low  =  95.0 / Device::SampleRate;
    constexpr double StopBand2High = 105.0 / Device::SampleRate;

    constexpr double StopBand3Low  = 145.0 / Device::SampleRate;
    constexpr double StopBand3High = 155.0 / Device::SampleRate;

    constexpr double LowPassCutoff = 180.0 / Device::SampleRate;

    constexpr auto StopBandWindow = SincFilter::WindowType::None;
    constexpr auto LowPassWindow  = SincFilter::WindowType::None;

    auto stopBand  = SincFilter::stopBand(FilterSize, StopBand1Low, StopBand1High, StopBandWindow);
    auto bandPass1 = SincFilter::bandPass(FilterSize, StopBand2Low, StopBand2High, StopBandWindow);
    auto bandPass2 = SincFilter::bandPass(FilterSize, StopBand3Low, StopBand3High, StopBandWindow);
    auto highPass  = SincFilter::highPass(FilterSize, LowPassCutoff, LowPassWindow);

    return stopBand - bandPass1 - bandPass2 - highPass;
}

// ---------------------------------------------------------------------------------------------- //
