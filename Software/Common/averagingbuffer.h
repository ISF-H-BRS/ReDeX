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

#include <array>
#include <cstddef>
#include <numeric>

// ---------------------------------------------------------------------------------------------- //

template <typename T, size_t N>
class AveragingBuffer
{
public:
    AveragingBuffer() = default;
    ~AveragingBuffer() noexcept = default;

    AveragingBuffer(const AveragingBuffer& other) = delete;
    AveragingBuffer(AveragingBuffer&& other) = delete;

    auto operator=(const AveragingBuffer& other) -> AveragingBuffer& = delete;
    auto operator=(AveragingBuffer&& other) noexcept -> AveragingBuffer& = delete;

    void addSample(const T& sample);
    auto getValue() const -> T;

    void clear();

private:
    std::array<T, N> m_data = {};
    size_t m_position = 0;
};

// ---------------------------------------------------------------------------------------------- //

template <typename T, size_t N>
void AveragingBuffer<T,N>::addSample(const T& sample)
{
    m_data[m_position] = sample;

    if (++m_position >= N)
        m_position = 0;
}

// ---------------------------------------------------------------------------------------------- //

template <typename T, size_t N>
auto AveragingBuffer<T,N>::getValue() const -> T
{
    return std::accumulate(m_data.begin(), m_data.end(), T()) / N;
}

// ---------------------------------------------------------------------------------------------- //

template <typename T, size_t N>
void AveragingBuffer<T,N>::clear()
{
    m_data = {};
    m_position = 0;
}

// ---------------------------------------------------------------------------------------------- //
