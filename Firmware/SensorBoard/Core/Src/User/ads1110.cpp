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

#include "ads1110.h"
#include "assert.h"

#include <array>

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr uint8_t DefaultConfigValue = 0x0c;
}

// ---------------------------------------------------------------------------------------------- //

namespace Offsets {
    constexpr uint8_t Start = 7;
    constexpr uint8_t Mode  = 4;
    constexpr uint8_t Rate  = 2;
    constexpr uint8_t Gain  = 0;
}

// ---------------------------------------------------------------------------------------------- //

ADS1110::ADS1110(Address address, Interface* interface)
    : m_interface(interface),
      m_address(static_cast<uint8_t>(address)),
      m_config(DefaultConfigValue)
{
}

// ---------------------------------------------------------------------------------------------- //

void ADS1110::setConfiguration(ConversionMode mode, DataRate rate, Gain gain)
{
    m_config = static_cast<uint8_t>(mode) << Offsets::Mode |
               static_cast<uint8_t>(rate) << Offsets::Rate |
               static_cast<uint8_t>(gain) << Offsets::Gain;

    uint8_t config[] = { m_config };
    write(config);
}

// ---------------------------------------------------------------------------------------------- //

void ADS1110::startConversion()
{
    uint8_t config[] = { static_cast<uint8_t>(m_config | (1 << Offsets::Start)) };
    write(config);
}

// ---------------------------------------------------------------------------------------------- //

auto ADS1110::readValue() const -> int16_t
{
    std::array<uint8_t, 2> buffer = {};
    read(buffer);

    uint16_t word = buffer[0] << 8 | buffer[1];

    return *reinterpret_cast<int16_t*>(&word);
}

// ---------------------------------------------------------------------------------------------- //

auto ADS1110::valueToReal(int16_t value) const -> double
{
    static_assert(DataRateCount == 4, "");

    static const std::array<int, DataRateCount> bits = { 12, 14, 15, 16 };

    const uint8_t rate = (m_config >> Offsets::Rate) & 0x03;
    return 1.0 / (1 << (bits[rate] - 1)) * value;
}

// ---------------------------------------------------------------------------------------------- //

void ADS1110::write(std::span<const uint8_t> buffer)
{
    m_interface->write(m_address, buffer);
}

// ---------------------------------------------------------------------------------------------- //

void ADS1110::read(std::span<uint8_t> buffer) const
{
    m_interface->read(m_address, buffer);
}

// ---------------------------------------------------------------------------------------------- //
