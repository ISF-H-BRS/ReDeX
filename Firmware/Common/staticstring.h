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

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <span>

// ---------------------------------------------------------------------------------------------- //

#ifndef ASSERT
#define ASSERT(x)
#endif

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
class StaticString
{
public:
    using value_type             = char;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_t Capacity = N;

    enum class TokenBehavior
    {
        KeepEmptyTokens,
        SkipEmptyTokens
    };

    static constexpr auto DefaultTokenBehavior = TokenBehavior::KeepEmptyTokens;

public:
    StaticString();
    StaticString(const StaticString& other);
    StaticString(StaticString&& other) noexcept;
    StaticString(const char* string);
    StaticString(const char* string, size_t first, size_t last);
    ~StaticString() noexcept = default;

    auto format(const char* format, ...) -> StaticString&;

    auto operator=(const StaticString& other) -> StaticString&;
    auto operator=(StaticString&& other) noexcept -> StaticString&;

    auto operator+(const StaticString& other) const -> StaticString;
    auto operator+(char c) const -> StaticString;

    auto operator+=(const StaticString& other) -> StaticString&;
    auto operator+=(char c) -> StaticString&;

    auto operator==(const StaticString& other) const -> bool;
    auto operator==(const char* other) const -> bool;

    auto operator!=(const StaticString& other) const -> bool;
    auto operator!=(const char* other) const -> bool;

    auto endsWith(const StaticString& string) const -> bool;
    auto endsWith(const char* string) const -> bool;
    auto endsWith(char c) const -> bool;

    auto trim(size_t count) -> StaticString&;
    auto trimmed(size_t count) const -> StaticString;

    auto getTokenCount(char separator,
                       TokenBehavior behavior = DefaultTokenBehavior) const -> size_t;

    auto getToken(char separator, size_t index,
                  TokenBehavior behavior = DefaultTokenBehavior) const -> StaticString;

    auto getTokens(char separator, size_t first, size_t count,
                   TokenBehavior behavior = DefaultTokenBehavior) const -> StaticString;

    auto getAllTokens(char separator, std::span<StaticString> tokens,
                      TokenBehavior behavior = DefaultTokenBehavior) const -> size_t;

    auto toInt() const -> int;
    auto toUInt() const -> unsigned int;
    auto toLong() const -> long;
    auto toULong() const -> unsigned long;
    auto toFloat() const -> float;
    auto toDouble() const -> double;

    void fill(char c);
    void clear();

    void push_back(char c);
    void pop_back();

    constexpr auto operator[](size_t n) noexcept -> char& { return m_data[n]; }
    constexpr auto operator[](size_t n) const noexcept -> const char& { return m_data[n]; }

    constexpr auto at(size_t n) -> char& { ASSERT(n < N); return m_data[n]; }
    constexpr auto at(size_t n) const -> const char& { ASSERT(n < N); return m_data[n]; }

    constexpr auto data() noexcept -> char* { return m_data.data(); }
    constexpr auto data() const noexcept -> const char* { return m_data.data(); }

    constexpr auto c_str() noexcept -> char* { return m_data.data(); }
    constexpr auto c_str() const noexcept -> const char* { return m_data.data(); }

    constexpr auto size() const -> size_t { return m_size; }
    constexpr auto capacity() const -> size_t { return N; }
    constexpr auto empty() const -> bool { return m_size == 0; }

    constexpr auto begin() -> char* { return m_data.begin(); }
    constexpr auto end() -> char* { return m_data.begin() + m_size; }

    constexpr auto rbegin() -> char* { return m_data.begin() + m_size - 1; }
    constexpr auto rend() -> char* { return m_data.begin() - 1; }

    constexpr auto cbegin() const -> const char* { return m_data.cbegin(); }
    constexpr auto cend() const -> const char* { return m_data.cbegin() + m_size; }

    constexpr auto crbegin() const -> const char* { return m_data.cbegin() + m_size - 1; }
    constexpr auto crend() const -> const char* { return m_data.cbegin() - 1; }

    static auto makeFormat(const char* format, ...) -> StaticString;

private:
    std::array<char, (N/4 + 1) * 4> m_data;
    size_t m_size = 0;
};

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
StaticString<N>::StaticString()
{
    m_data[0] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
StaticString<N>::StaticString(const StaticString& other)
    : m_size(other.m_size)
{
    std::copy_n(std::begin(other.m_data), other.m_size, std::begin(m_data));
    m_data[m_size] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
StaticString<N>::StaticString(StaticString&& other) noexcept
    : m_size(other.m_size)
{
    std::copy_n(std::begin(other.m_data), other.m_size, std::begin(m_data));
    m_data[m_size] = '\0';

    other.m_size = 0;
    other.m_data[0] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
StaticString<N>::StaticString(const char* string)
{
    while (*string != '\0' && m_size < N)
        m_data[m_size++] = *string++;

    m_data[m_size] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
StaticString<N>::StaticString(const char* string, size_t first, size_t last)
{
    for (size_t i = 0; i < first; ++i)
    {
        if (*string == '\0')
            break;

        ++string;
    }

    for (size_t i = first; i < last && m_size < N; ++i)
    {
        if (*string == '\0')
            break;

        m_data[m_size++] = *string++;
    }

    m_data[m_size] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::format(const char* format, ...) -> StaticString&
{
    va_list args;
    va_start(args, format);
    vsnprintf(m_data.data(), N, format, args);
    va_end(args);

    m_size = std::strlen(m_data.data());

    return *this;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator=(const StaticString& other) -> StaticString&
{
    std::copy_n(std::begin(other.m_data), other.m_size, std::begin(m_data));
    m_size = other.m_size;
    m_data[m_size] = '\0';

    return *this;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator=(StaticString&& other) noexcept -> StaticString&
{
    std::copy_n(std::begin(other.m_data), other.m_size, std::begin(m_data));
    m_size = other.m_size;
    m_data[m_size] = '\0';

    other.m_size = 0;
    other.m_data[0] = '\0';

    return *this;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator+(const StaticString& other) const -> StaticString
{
    StaticString result = *this;
    result += other;
    return result;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator+(char c) const -> StaticString
{
    StaticString result = *this;
    result += c;
    return result;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator+=(const StaticString& other) -> StaticString&
{
    ASSERT(m_size + other.m_size <= N);

    std::copy_n(std::begin(other.m_data), other.m_size, std::begin(m_data) + m_size);
    m_size += other.m_size;
    m_data[m_size] = '\0';

    return *this;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator+=(char c) -> StaticString&
{
    ASSERT(m_size < N);

    m_data[m_size++] = c;
    m_data[m_size] = '\0';

    return *this;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator==(const StaticString& other) const -> bool
{
    if (m_size != other.m_size)
        return false;

    for (size_t i = 0; i < m_size; ++i)
    {
        if (m_data[i] != other.m_data[i])
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator==(const char* other) const -> bool
{
    if (m_size != ::strlen(other))
        return false;

    for (size_t i = 0; i < m_size; ++i)
    {
        if (m_data[i] != other[i])
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator!=(const StaticString& other) const -> bool
{
    return !(*this == other);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::operator!=(const char* other) const -> bool
{
    return !(*this == other);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::endsWith(const StaticString& string) const -> bool
{
    if (string.m_size == 0 || string.m_size > m_size)
        return false;

    for (size_t i = 0; i < string.m_size; ++i)
    {
        if (string.m_data[i] != m_data[m_size - string.m_size + i])
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::endsWith(const char* string) const -> bool
{
    size_t strSize = ::strlen(string);

    if (strSize == 0 || strSize > m_size)
        return false;

    for (size_t i = 0; i < strSize; ++i)
    {
        if (string[i] != m_data[m_size - strSize + i])
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::endsWith(char c) const -> bool
{
    return m_size > 0 && m_data[m_size - 1] == c;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::trim(size_t count) -> StaticString&
{
    if (count > m_size)
        count = m_size;

    m_size -= count;
    m_data[m_size] = '\0';

    return *this;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::trimmed(size_t count) const -> StaticString
{
    if (count > m_size)
        count = m_size;

    return StaticString(m_data, 0, count);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::getTokenCount(char separator, TokenBehavior behavior) const -> size_t
{
    if (m_size == 0)
        return 0;

    if (behavior == TokenBehavior::KeepEmptyTokens)
    {
        size_t count = 1;

        for (size_t i = 0; i < m_size; ++i)
        {
            if (m_data[i] == separator)
                ++count;
        }

        return count;
    }

    size_t count = 0;
    size_t size = 0;

    for (size_t i = 0; i < m_size; ++i)
    {
        if (m_data[i] == separator)
        {
            if (size > 0)
            {
                ++count;
                size = 0;
            }
        }
        else
            ++size;
    }

    if (size > 0)
        ++count;

    return count;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::getToken(char separator, size_t index,
                               TokenBehavior behavior) const -> StaticString
{
    return getTokens(separator, index, 1, behavior);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::getTokens(char separator, size_t first, size_t count,
                                TokenBehavior behavior) const -> StaticString
{
    ASSERT(count > 0);

    StaticString result;

    size_t index = 0;
    size_t size = 0;

    for (size_t i = 0; i < m_size; ++i)
    {
        if (m_data[i] == separator)
        {
            if (behavior == TokenBehavior::KeepEmptyTokens || size > 0)
            {
                if (index >= (first + count - 1))
                    break;

                if (index >= first)
                    result += m_data[i];

                ++index;
                size = 0;
            }
        }
        else
        {
            if (index >= first)
                result += m_data[i];

            ++size;
        }
    }

    return result;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::getAllTokens(char separator, std::span<StaticString> tokens,
                                   TokenBehavior behavior) const -> size_t
{
    size_t index = 0;
    size_t size = 0;

    for (size_t i = 0; i < m_size; ++i)
    {
        if (m_data[i] == separator)
        {
            if (behavior == TokenBehavior::KeepEmptyTokens || size > 0)
            {
                ++index;
                size = 0;
            }
        }
        else
        {
            ASSERT(index < tokens.size());
            tokens[index] += m_data[i];

            ++size;
        }
    }

    if (behavior == TokenBehavior::KeepEmptyTokens || size > 0)
        ++index;

    return index;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::toInt() const -> int
{
    return std::strtol(m_data.data(), nullptr, 0);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::toUInt() const -> unsigned int
{
    return std::strtoul(m_data.data(), nullptr, 0);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::toLong() const -> long
{
    return std::strtol(m_data.data(), nullptr, 0);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::toULong() const -> unsigned long
{
    return std::strtoul(m_data.data(), nullptr, 0);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::toFloat() const -> float
{
    return std::strtof(m_data.data(), nullptr);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::toDouble() const -> double
{
    return std::strtod(m_data.data(), nullptr);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
void StaticString<N>::fill(char c)
{
    std::fill(std::begin(m_data), m_size, c);
    m_data[m_size] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
void StaticString<N>::clear()
{
    m_size = 0;
    m_data[m_size] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
void StaticString<N>::push_back(char c)
{
    ASSERT(m_size < N);

    m_data[m_size++] = c;
    m_data[m_size] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
void StaticString<N>::pop_back()
{
    --m_size;
    m_data[m_size] = '\0';
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto StaticString<N>::makeFormat(const char* format, ...) -> StaticString
{
    char buffer[N + 1];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, N, format, args);
    va_end(args);

    return StaticString(buffer);
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto operator+(const char* str1, const StaticString<N>& str2) -> StaticString<N>
{
    return StaticString<N>(str1) + str2;
}

// ---------------------------------------------------------------------------------------------- //

template <size_t N>
auto operator<<(std::ostream& os, const StaticString<N>& string) -> std::ostream&
{
    return os << string.data();
}

// ---------------------------------------------------------------------------------------------- //
