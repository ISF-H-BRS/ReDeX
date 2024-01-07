/****************************************************************
 *                                                              *
 *  This file is part of the ISF SkinCam project.               *
 *                                                              *
 *  Author:                                                     *
 *  Marcel Hasler <marcel.hasler@h-brs.de>                      *
 *                                                              *
 *  (c) 2021                                                    *
 *  Bonn-Rhein-Sieg University of Applied Sciences              *
 *                                                              *
 *  All Rights Reserved. Redistribution is only permitted       *
 *  with express written permission from the copyright holder.  *
 *                                                              *
 ****************************************************************/

#pragma once

#include "assert.h"
#include "config.h"

#include <chrono>
using namespace std::chrono_literals;

class Delay
{
public:
    static constexpr auto MaximumWait = std::chrono::microseconds((1<<16) - 1);

public:
    static void init()
    {
        HAL_TIM_Base_Start(Config::DelayTimerHandle);
    }

    template <typename Predicate>
    static auto wait(std::chrono::microseconds us, Predicate pred) -> bool
    {
        ASSERT(us <= MaximumWait);

        uint32_t count = 0;
        uint16_t last = now();

        while (count < us.count())
        {
            if (pred())
                return true;

            uint16_t current = now();
            uint16_t diff = current - last;

            count += diff;
            last = current;
        }

        return false;
    }

    static void wait(std::chrono::microseconds us)
    {
        wait(us, []{ return false; });
    }

private:
    static inline auto now() -> uint16_t
    {
        return __HAL_TIM_GET_COUNTER(Config::DelayTimerHandle);
    }
};
