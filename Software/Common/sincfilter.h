/**************************************************************************************************
 *                                                                                                *
 *  This file is part of the ISF Utilities collection.                                            *
 *                                                                                                *
 *  Author:                                                                                       *
 *  Marcel Hasler <marcel.hasler@h-brs.de>                                                        *
 *                                                                                                *
 *  Copyright (c) 2022                                                                            *
 *  Bonn-Rhein-Sieg University of Applied Sciences                                                *
 *                                                                                                *
 *  Redistribution and use in source and binary forms, with or without modification,              *
 *  are permitted provided that the following conditions are met:                                 *
 *                                                                                                *
 *  1. Redistributions of source code must retain the above copyright notice,                     *
 *     this list of conditions and the following disclaimer.                                      *
 *                                                                                                *
 *  2. Redistributions in binary form must reproduce the above copyright notice,                  *
 *     this list of conditions and the following disclaimer in the documentation                  *
 *     and/or other materials provided with the distribution.                                     *
 *                                                                                                *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"                   *
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED             *
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.            *
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,              *
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT            *
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR            *
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,             *
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)            *
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE                    *
 *  POSSIBILITY OF SUCH DAMAGE.                                                                   *
 *                                                                                                *
 **************************************************************************************************/

#pragma once

#include <span>
#include <vector>

class SincFilter
{
public:
    static constexpr size_t MinimumSize = 3;

    static constexpr double MinimumFrequency = 0.0;
    static constexpr double MaximumFrequency = 0.5;

    enum class WindowType
    {
        None,
        Blackman,
        Hamming
    };

    static constexpr auto DefaultWindowType = WindowType::None;

public:
    auto size() const -> size_t;
    auto coefficients() const -> const std::vector<double>&;

    void apply(std::span<const double> input, std::span<double> output, double scale = 1.0);

    auto operator+(const SincFilter& rhs) const -> SincFilter;
    auto operator-(const SincFilter& rhs) const -> SincFilter;
    auto operator*(const SincFilter& rhs) const -> SincFilter;
    auto operator~() const -> SincFilter;

    static auto lowPass(size_t size, double fc,
                        WindowType window = DefaultWindowType) -> SincFilter;

    static auto highPass(size_t size, double fc,
                         WindowType window = DefaultWindowType) -> SincFilter;

    static auto stopBand(size_t size, double fl, double fh,
                         WindowType window = DefaultWindowType) -> SincFilter;

    static auto bandPass(size_t size, double fl, double fh,
                         WindowType window = DefaultWindowType) -> SincFilter;
private:
    SincFilter(size_t size);
    SincFilter(size_t size, double fc, WindowType window);

    void blackman();
    void hamming();

    void normalize();

    static void convolve(std::span<const double> in1, std::span<const double> in2,
                         std::span<double> out);

private:
    std::vector<double> m_coeffs;
};
