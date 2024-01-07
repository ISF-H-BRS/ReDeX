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
#include "max521x.h"

// ---------------------------------------------------------------------------------------------- //

Max521x::Max521x(Interface* interface, Address address)
    : m_interface(interface),
      m_address(static_cast<uint8_t>(address) << 1)
{
    ASSERT(interface != nullptr);
}

// ---------------------------------------------------------------------------------------------- //

void Max521x::setValue(uint16_t value)
{
    static constexpr uint8_t CommandCodeLoad = 0x01;

    ASSERT(value <= maximumValue());
    value = shiftValue(value);

    std::array<uint8_t, 3> data = {
        CommandCodeLoad,
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value)
    };

    m_interface->write(m_address, data);
}

// ---------------------------------------------------------------------------------------------- //

Max5215::Max5215(Interface* interface, Address address)
    : Max521x(interface, address) {}

// ---------------------------------------------------------------------------------------------- //

auto Max5215::maximumValue() const -> uint16_t
{
    return MaximumValue;
}

// ---------------------------------------------------------------------------------------------- //

auto Max5215::shiftValue(uint16_t value) const -> uint16_t
{
    return value << 2;
}

// ---------------------------------------------------------------------------------------------- //

Max5217::Max5217(Interface* interface, Address address)
    : Max521x(interface, address) {}

// ---------------------------------------------------------------------------------------------- //

auto Max5217::maximumValue() const -> uint16_t
{
    return MaximumValue;
}

// ---------------------------------------------------------------------------------------------- //

auto Max5217::shiftValue(uint16_t value) const -> uint16_t
{
    return value;
}

// ---------------------------------------------------------------------------------------------- //
