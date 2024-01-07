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

#include "sincfilter.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr double Pi = 3.14159265358979323846;
    constexpr double TwoPi = 2.0 * Pi;
    constexpr double FourPi = 4.0 * Pi;
}

// ---------------------------------------------------------------------------------------------- //

SincFilter::SincFilter(size_t size)
    : m_coeffs(size, 0.0)
{
    assert(size >= MinimumSize && size % 2 == 1);
}

// ---------------------------------------------------------------------------------------------- //

SincFilter::SincFilter(size_t ssize, double fc, WindowType window)
    : SincFilter(ssize)
{
    assert(fc > MinimumFrequency && fc < MaximumFrequency);

    const auto size = static_cast<int>(m_coeffs.size());
    const int shift = (size - 1) / 2;

    for (int i = 0; i < size; ++i)
    {
        if (i == shift)
            m_coeffs[i] = TwoPi * fc;
        else
            m_coeffs[i] = std::sin(TwoPi * fc * (i - shift)) / (i - shift);
    }

    if (window == WindowType::Blackman)
        blackman();
    else if (window == WindowType::Hamming)
        hamming();

    normalize();
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::size() const -> size_t
{
    return m_coeffs.size();
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::coefficients() const -> const std::vector<double>&
{
    return m_coeffs;
}

// ---------------------------------------------------------------------------------------------- //

void SincFilter::apply(std::span<const double> input, std::span<double> output, double scale)
{
    convolve(input, m_coeffs, output);

    if (scale != 1.0)
    {
        std::transform(output.begin(), output.end(), output.begin(),
                       [scale](double c) { return scale*c; });
    }
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::operator+(const SincFilter& rhs) const -> SincFilter
{
    assert(rhs.size() == m_coeffs.size());

    SincFilter out(m_coeffs.size());

    std::transform(m_coeffs.begin(), m_coeffs.end(), rhs.m_coeffs.begin(), out.m_coeffs.begin(),
                   std::plus());
    return out;
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::operator-(const SincFilter& rhs) const -> SincFilter
{
    assert(rhs.size() == m_coeffs.size());

    SincFilter out(m_coeffs.size());

    std::transform(m_coeffs.begin(), m_coeffs.end(), rhs.m_coeffs.begin(), out.m_coeffs.begin(),
                   std::minus());
    return out;
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::operator*(const SincFilter& rhs) const -> SincFilter
{
    SincFilter out(m_coeffs.size() + rhs.size() - 1);
    convolve(m_coeffs, rhs.m_coeffs, out.m_coeffs);
    return out;
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::operator~() const -> SincFilter
{
    SincFilter out(m_coeffs.size());

    std::transform(m_coeffs.begin(), m_coeffs.end(), out.m_coeffs.begin(), std::negate());
    out.m_coeffs[(out.m_coeffs.size() - 1) / 2] += 1.0;

    return out;
}

// ---------------------------------------------------------------------------------------------- //

void SincFilter::blackman()
{
    const auto size = static_cast<int>(m_coeffs.size());
    const int m = size - 1;

    for (int i = 0; i < size; ++i)
        m_coeffs[i] *= 0.42 - 0.5*std::cos(TwoPi*i / m) + 0.08*std::cos(FourPi*i / m);
}

// ---------------------------------------------------------------------------------------------- //

void SincFilter::hamming()
{
    const auto size = static_cast<int>(m_coeffs.size());
    const int m = size - 1;

    for (int i = 0; i < size; ++i)
        m_coeffs[i] *= 0.54 - 0.46*std::cos(TwoPi*i / m);
}

// ---------------------------------------------------------------------------------------------- //

void SincFilter::normalize()
{
    const double sum = std::accumulate(m_coeffs.begin(), m_coeffs.end(), 0.0);

    std::transform(m_coeffs.begin(), m_coeffs.end(), m_coeffs.begin(),
                   [sum](double c) { return c /= sum; });
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::lowPass(size_t size, double fc, WindowType window) -> SincFilter
{
    return SincFilter(size, fc, window);
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::highPass(size_t size, double fc, WindowType window) -> SincFilter
{
    return ~lowPass(size, fc, window);
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::stopBand(size_t size, double fl, double fh, WindowType window) -> SincFilter
{
    assert(fl < fh);
    return lowPass(size, fl, window) + highPass(size, fh, window);
}

// ---------------------------------------------------------------------------------------------- //

auto SincFilter::bandPass(size_t size, double fl, double fh, WindowType window) -> SincFilter
{
    return ~stopBand(size, fl, fh, window);
}

// ---------------------------------------------------------------------------------------------- //

void SincFilter::convolve(std::span<const double> in1, std::span<const double> in2,
                          std::span<double> out)
{
    assert(out.size() >= in1.size() + in2.size() - 1);

    std::fill(out.begin(), out.end(), 0.0);

    for (size_t i = 0; i < in1.size(); ++i)
    {
        for (size_t j = 0; j < in2.size(); ++j)
            out[i + j] += in1[i] * in2[j];
    }
}

// ---------------------------------------------------------------------------------------------- //
