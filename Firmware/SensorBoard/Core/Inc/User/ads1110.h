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

class ADS1110
{
public:
    class Interface
    {
    public:
        virtual void read(uint8_t address, std::span<uint8_t> data) const = 0;
        virtual void write(uint8_t address, std::span<const uint8_t> data) const = 0;
    };

    enum class Address : uint8_t
    {
        A0 = (0x48 << 1),
        A1 = (0x49 << 1),
        A2 = (0x4a << 1),
        A3 = (0x4b << 1),
        A4 = (0x4c << 1),
        A5 = (0x4d << 1),
        A6 = (0x4e << 1),
        A7 = (0x4f << 1)
    };

    static constexpr size_t AddressCount = 8;

    enum class ConversionMode : uint8_t
    {
        Continuous = 0,
        Single
    };

    static constexpr size_t ConversionModeCount = 2;

    enum class DataRate : uint8_t
    {
        _240Hz = 0,
        _60Hz,
        _30Hz,
        _15Hz
    };

    static constexpr size_t DataRateCount = 4;

    enum class Gain : uint8_t
    {
        _1x = 0,
        _2x,
        _4x,
        _8x
    };

    static constexpr size_t GainCount = 4;

public:
    ADS1110(Address address, Interface* interface);

    void setConfiguration(ConversionMode mode, DataRate rate, Gain gain);

    void startConversion();

    auto readValue() const -> int16_t;

    auto valueToReal(int16_t value) const -> double;

private:
    void write(std::span<const uint8_t> buffer);
    void read(std::span<uint8_t> buffer) const;

private:
    Interface* m_interface;
    uint8_t m_address;
    uint8_t m_config;
};
