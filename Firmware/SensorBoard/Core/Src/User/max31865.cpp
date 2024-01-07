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

#include "assert.h"
#include "max31865.h"

#include <algorithm>
#include <array>

// ---------------------------------------------------------------------------------------------- //

namespace Register {
    constexpr uint8_t Configuration  = 0x00;
    constexpr uint8_t ResistanceHigh = 0x01;
    constexpr uint8_t ResistanceLow  = 0x02;
}

namespace Configuration {
    constexpr uint8_t VBiasOffset          = 7;
    constexpr uint8_t ConversionModeOffset = 6;
    constexpr uint8_t OneShotOffset        = 5;
    constexpr uint8_t ThreeWireOffset      = 4;
    constexpr uint8_t FaultDetectionOffset = 3;
    constexpr uint8_t FaultClearOffset     = 1;
    constexpr uint8_t FilterSelectOffset   = 0;
}

// ---------------------------------------------------------------------------------------------- //

MAX31865::MAX31865(const Setup& setup, Interface* interface)
    : m_interface(interface)
{
    if (setup.connection == ConnectionMode::ThreeWire)
        m_config |= (1 << Configuration::ThreeWireOffset);

    if (setup.filter == FilterSelect::_50Hz)
        m_config |= (1 << Configuration::FilterSelectOffset);

    writeConfig(m_config);
}

// ---------------------------------------------------------------------------------------------- //

void MAX31865::setBiasVoltageEnabled(bool enable)
{
    if (enable)
        m_config |= (1 << Configuration::VBiasOffset);
    else
        m_config &= ~(1 << Configuration::VBiasOffset);

    writeConfig(m_config);
}

// ---------------------------------------------------------------------------------------------- //

auto MAX31865::biasVoltageEnabled() const -> bool
{
    return m_config & (1 << Configuration::VBiasOffset);
}

// ---------------------------------------------------------------------------------------------- //


void MAX31865::setAutoConversionEnabled(bool enable)
{
    if (enable)
        m_config |= (1 << Configuration::ConversionModeOffset);
    else
        m_config &= ~(1 << Configuration::ConversionModeOffset);

    writeConfig(m_config);
}

// ---------------------------------------------------------------------------------------------- //

auto MAX31865::autoConversionEnabled() const -> bool
{
    return m_config & (1 << Configuration::ConversionModeOffset);
}

// ---------------------------------------------------------------------------------------------- //

void MAX31865::runSingleConversion()
{
    writeConfig(m_config | Configuration::OneShotOffset);
}

// ---------------------------------------------------------------------------------------------- //

auto MAX31865::getRawValue() const -> uint16_t
{
    const auto dataReady = [this]
    {
        for (size_t i = 0; i < DataReadyRetries; ++i)
        {
            if (m_interface->dataReady())
                return true;
        }

        return false;
    };

    if (!dataReady())
        throw TimeoutError();

    std::array<uint8_t, 2> values;
    read(Register::ResistanceHigh, values);

    const uint16_t value = (values[0] << 8) | values[1];
    return value >> 1; // Remove fault bit
}

// ---------------------------------------------------------------------------------------------- //

auto MAX31865::getRelativeValue() const -> double
{
    return static_cast<double>(getRawValue()) / (1<<15);
}

// ---------------------------------------------------------------------------------------------- //

void MAX31865::read(uint8_t address, std::span<uint8_t> data) const
{
    ASSERT(data.size() <= MaximumDataSize);

    m_writeBuffer[0] = address;

    std::fill_n(m_writeBuffer.begin() + 1, data.size(), 0x00);
    std::fill_n(m_readBuffer.begin(), data.size() + 1, 0x00);

    m_interface->swap({ m_writeBuffer.data(), data.size() + 1 },
                      { m_readBuffer.data(),  data.size() + 1 });

    std::copy_n(m_readBuffer.begin() + 1, data.size(), data.begin());
}

// ---------------------------------------------------------------------------------------------- //

void MAX31865::write(uint8_t address, std::span<const uint8_t> data) const
{
    ASSERT(data.size() <= MaximumDataSize);

    static constexpr uint8_t WriteMask = 0x80;

    m_writeBuffer[0] = address | WriteMask;
    std::copy_n(data.begin(), data.size(), m_writeBuffer.begin() + 1);

    m_interface->write({ m_writeBuffer.data(), data.size() + 1 });
}

// ---------------------------------------------------------------------------------------------- //

auto MAX31865::readConfig() const -> uint8_t
{
    uint8_t config;
    read(Register::Configuration, { &config, 1 });

    return config;
}

// ---------------------------------------------------------------------------------------------- //

void MAX31865::writeConfig(uint8_t config) const
{
    write(Register::Configuration, { &config, 1 });
}

// ---------------------------------------------------------------------------------------------- //
