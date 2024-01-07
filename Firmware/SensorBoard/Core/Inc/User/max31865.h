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

class MAX31865
{
public:
    class Interface
    {
    public:
        virtual void write(std::span<const uint8_t> data) const = 0;
        virtual void swap(std::span<const uint8_t> out, std::span<uint8_t> in) const = 0;
        virtual auto dataReady() const -> bool = 0;
    };

    static constexpr size_t DataReadyRetries = 100;

    enum class ConnectionMode
    {
        TwoOrFourWire,
        ThreeWire
    };

    enum class FilterSelect
    {
        _60Hz,
        _50Hz
    };

    struct Setup
    {
        ConnectionMode connection = ConnectionMode::TwoOrFourWire;
        FilterSelect filter = FilterSelect::_60Hz;
    };

    class TimeoutError : public std::exception
    {
    public:
        auto what() const noexcept -> const char* override { return "MAX31865_TIMEOUT"; }
    };

public:
    MAX31865(const Setup& setup, Interface* interface);

    void setBiasVoltageEnabled(bool enable);
    auto biasVoltageEnabled() const -> bool;

    void setAutoConversionEnabled(bool enable);
    auto autoConversionEnabled() const -> bool;

    void runSingleConversion();

    auto getRawValue() const -> uint16_t;
    auto getRelativeValue() const -> double;

private:
    void read(uint8_t address, std::span<uint8_t> data) const;
    void write(uint8_t address, std::span<const uint8_t> data) const;

    auto readConfig() const -> uint8_t;
    void writeConfig(uint8_t config) const;

private:
    Interface* m_interface;
    uint8_t m_config = 0x00;

    static constexpr size_t MaximumDataSize = 2;
    mutable std::array<uint8_t, MaximumDataSize + 1> m_writeBuffer = {};
    mutable std::array<uint8_t, MaximumDataSize + 1> m_readBuffer = {};
};
