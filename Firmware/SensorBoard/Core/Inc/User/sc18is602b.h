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

#include <cstdint>
#include <span>
#include <tuple>

class SC18IS602B
{
public:
    class Interface
    {
    public:
        virtual void read(uint8_t address, std::span<uint8_t> data) const = 0;
        virtual void write(uint8_t address, std::span<const uint8_t> data) const = 0;
    };

    enum class Address
    {
        A0 = 0x50,
        A1 = 0x52,
        A2 = 0x54,
        A3 = 0x56,
        A4 = 0x58,
        A5 = 0x5a,
        A6 = 0x5c,
        A7 = 0x5e
    };

    enum class BitOrder
    {
        MsbFirst,
        LsbFirst
    };

    enum class SpiMode
    {
        IdleLowLeadingEdge,
        IdleLowTrailingEdge,
        IdleHighLeadingEdge,
        IdleHighTrailingEdge
    };

    enum class ClockRate
    {
        _1843kHz,
        _461kHz,
        _115kHz,
        _58kHz
    };

    enum class GpioConfiguration
    {
        QuasiBidirectional,
        PushPull,
        InputOnly,
        OpenDrain
    };

    enum class SlaveSelect
    {
        SS0,
        SS1,
        SS2,
        SS3
    };

    template <typename T = uint8_t>
    static constexpr auto indexOf(SlaveSelect ss) { return static_cast<T>(ss); }

    struct Setup
    {
        Address address = Address::A0;
        BitOrder bitOrder = BitOrder::MsbFirst;
        SpiMode spiMode = SpiMode::IdleLowLeadingEdge;
        ClockRate clockRate = ClockRate::_1843kHz;
    };

    static constexpr size_t MaximumDataSize = 200;

public:
    SC18IS602B(const Setup& setup, Interface* interface);

    void setGpioEnabled(bool ss0, bool ss1, bool ss2, bool ss3);

    void setGpioConfiguration(GpioConfiguration ss0, GpioConfiguration ss1,
                              GpioConfiguration ss2, GpioConfiguration ss3);

    void setGpioValue(bool ss0, bool ss1, bool ss2, bool ss3);

    auto getGpioValue() const -> std::tuple<bool,bool,bool,bool>;
    auto getGpioValue(SlaveSelect ss) const -> bool;

    void readData(SlaveSelect ss, std::span<uint8_t> data) const;
    void writeData(SlaveSelect ss, std::span<const uint8_t> data) const;
    void swapData(SlaveSelect ss, std::span<const uint8_t> out, std::span<uint8_t> in) const;

private:
    void read(std::span<uint8_t> data) const;
    void write(uint8_t function, std::span<const uint8_t> data) const;

    auto readGpioValue() const -> uint8_t;

private:
    Address m_address;
    Interface* m_interface;

    mutable std::array<uint8_t, MaximumDataSize + 1> m_buffer = {};
};
