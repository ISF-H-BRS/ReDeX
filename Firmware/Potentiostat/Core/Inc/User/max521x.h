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

// ---------------------------------------------------------------------------------------------- //

class Max521x
{
public:
    enum class Address : uint8_t
    {
        A0 = 0b0011100,
        A1 = 0b0011101,
        A2 = 0b0011111
    };

    class Interface
    {
        friend class Max521x;
        virtual void write(uint8_t address, std::span<const uint8_t> data) = 0;
    };

public:
    void setValue(uint16_t value);

protected:
    Max521x(Interface* interface, Address address);

    virtual auto maximumValue() const -> uint16_t = 0;
    virtual auto shiftValue(uint16_t value) const -> uint16_t = 0;

private:
    Interface* m_interface;
    uint8_t m_address;
};

// ---------------------------------------------------------------------------------------------- //

class Max5215 : public Max521x
{
public:
    static constexpr uint16_t MaximumValue = (1<<14) - 1;

public:
    Max5215(Interface* interface, Address address);

private:
    auto maximumValue() const -> uint16_t override;
    auto shiftValue(uint16_t value) const -> uint16_t override;
};

// ---------------------------------------------------------------------------------------------- //

class Max5217 : public Max521x
{
public:
    static constexpr uint16_t MaximumValue = (1<<16) - 1;

public:
    Max5217(Interface* interface, Address address);

private:
    auto maximumValue() const -> uint16_t override;
    auto shiftValue(uint16_t value) const -> uint16_t override;
};

// ---------------------------------------------------------------------------------------------- //
