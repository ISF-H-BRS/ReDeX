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

#include "assert.h"
#include "nonesensor.h"
#include "phsensor.h"
#include "temperaturesensor.h"

#include <array>

// ---------------------------------------------------------------------------------------------- //

namespace SensorMemPoolInternal {

constexpr inline size_t maxSensorSize()
{
    size_t size = sizeof(uintptr_t);

    if (sizeof(PhSensor) > size)
        size = sizeof(PhSensor);

    if (sizeof(TemperatureSensor) > size)
        size = sizeof(TemperatureSensor);

    if (sizeof(NoneSensor) > size)
        size = sizeof(NoneSensor);

    return size;
}

} // End of namespace SensorMemPoolInternal

// ---------------------------------------------------------------------------------------------- //

class SensorMemPool
{
public:
    static constexpr uint8_t MaxSensorCount = 2;
    static constexpr size_t MaxSensorSize = SensorMemPoolInternal::maxSensorSize();

public:
    auto getNextFreeBuffer() -> char*
    {
        ASSERT(m_usedBuffers < MaxSensorCount);

        char* buffer = &m_pool[m_usedBuffers * MaxSensorSize];
        ++m_usedBuffers;

        return buffer;
    }

private:
    std::array<char, MaxSensorCount * MaxSensorSize> m_pool = {};
    size_t m_usedBuffers = 0;
};

// ---------------------------------------------------------------------------------------------- //
