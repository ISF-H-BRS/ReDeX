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
#include "sc18is602b.h"

// ---------------------------------------------------------------------------------------------- //

namespace Function {
    constexpr uint8_t ConfigureInterface = 0xf0;
    constexpr uint8_t GpioWrite          = 0xf4;
    constexpr uint8_t GpioRead           = 0xf5;
    constexpr uint8_t GpioEnable         = 0xf6;
}

// ---------------------------------------------------------------------------------------------- //

SC18IS602B::SC18IS602B(const Setup& setup, Interface* interface)
    : m_address(setup.address),
      m_interface(interface)
{
    static constexpr uint8_t OrderOffset = 5;
    static constexpr uint8_t ModeOffset  = 3;
    static constexpr uint8_t RateOffset  = 0;

    const uint8_t value = (static_cast<uint8_t>(setup.bitOrder)  << OrderOffset) |
                          (static_cast<uint8_t>(setup.spiMode)   << ModeOffset)  |
                          (static_cast<uint8_t>(setup.clockRate) << RateOffset);

    write(Function::ConfigureInterface, { &value, 1 });
}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::setGpioEnabled(bool ss0, bool ss1, bool ss2, bool ss3)
{
    const uint8_t mask = (static_cast<uint8_t>(ss0) << 0) |
                         (static_cast<uint8_t>(ss1) << 1) |
                         (static_cast<uint8_t>(ss2) << 2) |
                         (static_cast<uint8_t>(ss3) << 3);

    write(Function::GpioEnable, { &mask, 1 });
}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::setGpioConfiguration(GpioConfiguration ss0, GpioConfiguration ss1,
                                      GpioConfiguration ss2, GpioConfiguration ss3)
{

}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::setGpioValue(bool ss0, bool ss1, bool ss2, bool ss3)
{
    const uint8_t mask = (static_cast<uint8_t>(ss0) << 0) |
                         (static_cast<uint8_t>(ss1) << 1) |
                         (static_cast<uint8_t>(ss2) << 2) |
                         (static_cast<uint8_t>(ss3) << 3);

    write(Function::GpioWrite, { &mask, 1 });
}

// ---------------------------------------------------------------------------------------------- //

auto SC18IS602B::getGpioValue() const -> std::tuple<bool,bool,bool,bool>
{
    const uint8_t mask = readGpioValue();

    const bool ss0 = mask & (1<<0);
    const bool ss1 = mask & (1<<1);
    const bool ss2 = mask & (1<<2);
    const bool ss3 = mask & (1<<3);

    return { ss0, ss1, ss2, ss3 };
}

// ---------------------------------------------------------------------------------------------- //

auto SC18IS602B::getGpioValue(SlaveSelect ss) const -> bool
{
    const uint8_t mask = readGpioValue();
    return mask & (1 << indexOf(ss));
}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::readData(SlaveSelect ss, std::span<uint8_t> data) const
{
    const uint8_t function = (1 << indexOf(ss));
    write(function, data);
    read(data);
}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::writeData(SlaveSelect ss, std::span<const uint8_t> data) const
{
    const uint8_t function = (1 << indexOf(ss));
    write(function, data);
}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::swapData(SlaveSelect ss, std::span<const uint8_t> out, std::span<uint8_t> in) const
{
    const uint8_t function = (1 << indexOf(ss));
    write(function, out);
    read(in);
}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::read(std::span<uint8_t> data) const
{
    ASSERT(data.size() <= MaximumDataSize);
    m_interface->read(static_cast<uint8_t>(m_address), data);
}

// ---------------------------------------------------------------------------------------------- //

void SC18IS602B::write(uint8_t function, std::span<const uint8_t> data) const
{
    ASSERT(data.size() <= MaximumDataSize);

    m_buffer[0] = function;
    std::copy(data.begin(), data.end(), m_buffer.begin() + 1);

    m_interface->write(static_cast<uint8_t>(m_address), { m_buffer.data(), data.size() + 1 });
}

// ---------------------------------------------------------------------------------------------- //

auto SC18IS602B::readGpioValue() const -> uint8_t
{
    const uint8_t dummy = {};
    write(Function::GpioRead, { &dummy, 1 });

    uint8_t mask = {};
    read({ &mask, 1 });

    return mask;
}

// ---------------------------------------------------------------------------------------------- //
