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

#include <conductance/databuffer.h>

#include <cassert>

// ---------------------------------------------------------------------------------------------- //

using namespace isf::Conductance;

// ---------------------------------------------------------------------------------------------- //

DataBuffer::DataBuffer(Device::Input input)
    : m_input(input)
{
    m_offset.resize(Device::SampleRate + 1);

    for (unsigned int i = 0; i <= Device::SampleRate; ++i)
        m_offset[i] = 1.0 / Device::SampleRate * i;

    m_voltage.resize(Device::SampleRate + 1);
    m_current.resize(Device::SampleRate + 1);
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::input() const -> Device::Input
{
    return m_input;
}

// ---------------------------------------------------------------------------------------------- //

void DataBuffer::update(const Device::Data& data)
{
    static constexpr auto VoltageIndex = Device::indexOf(Device::Channel::Voltage);
    static constexpr auto CurrentIndex = Device::indexOf(Device::Channel::Current);

    const auto input = Device::indexOf(m_input);

    for (size_t sample = 0; sample < Device::SamplesPerTransfer; ++sample)
    {
        m_voltage[m_dataPosition] = data[input][VoltageIndex][sample];
        m_current[m_dataPosition] = data[input][CurrentIndex][sample];

        if (++m_dataPosition >= Device::SampleRate)
            m_dataPosition = 0;
    }

    for (size_t input = 0; input < Device::InputCount; ++input)
    {
        m_voltage.back() = m_voltage.front();
        m_current.back() = m_current.front();
   }
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::offset() -> DataVector&
{
    return m_offset;
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::offset() const -> const DataVector&
{
    return m_offset;
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::voltage() -> DataVector&
{
    return m_voltage;
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::voltage() const -> const DataVector&
{
    return m_voltage;
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::current() -> DataVector&
{
    return m_current;
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::current() const -> const DataVector&
{
    return m_current;
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::data(Device::Channel channel) -> DataVector&
{
    return (channel == Device::Channel::Voltage) ? m_voltage : m_current;
}

// ---------------------------------------------------------------------------------------------- //

auto DataBuffer::data(Device::Channel channel) const -> const DataVector&
{
    return (channel == Device::Channel::Voltage) ? m_voltage : m_current;
}

// ---------------------------------------------------------------------------------------------- //

void DataBuffer::clear()
{
    std::fill(m_voltage.begin(), m_voltage.end(), 0.0);
    std::fill(m_current.begin(), m_current.end(), 0.0);
}

// ---------------------------------------------------------------------------------------------- //
