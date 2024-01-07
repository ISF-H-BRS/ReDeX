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

#include "delay.h"
#include "main.h"
#include "potentiostat.h"
#include "terminate.h"
#include "usermain.h"

#include <array>
#include <new>

// ---------------------------------------------------------------------------------------------- //

namespace __gnu_cxx
{
    // Strip unused name-demangling code from libstdc++
    void __verbose_terminate_handler() { TERMINATE(); }
}

// ---------------------------------------------------------------------------------------------- //

namespace {
    std::array<char, sizeof(Potentiostat)> g_buffer;
}

// ---------------------------------------------------------------------------------------------- //

void user_main()
{
    Delay::init();
    Delay::wait(50ms); // Wait for power to stabilize

    Potentiostat* potentiostat = nullptr;

    try {
        potentiostat = new (g_buffer.data()) Potentiostat;
        potentiostat->exec();
    }
    catch (...)
    {
        if (potentiostat)
            potentiostat->~Potentiostat();

        TERMINATE();
    }
}

// ---------------------------------------------------------------------------------------------- //