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

#include "device.h"
#include "sincfilter.h"

#include <deque>
#include <span>
#include <vector>

class SignalFilter
{
public:
    using InputBuffer = std::span<const double>;
    using OutputBuffer = std::vector<double>;

public:
    void reset();
    void update(InputBuffer input, OutputBuffer* output);
    void finalize(OutputBuffer* output);

private:
    void process(double input, OutputBuffer* output);

    static auto makeFilter() -> SincFilter;

private:
    const SincFilter m_filter = makeFilter();
    const size_t m_filterSize = m_filter.size();

    std::deque<double> m_buffer;
    size_t m_pendingSamples = 0;
};
