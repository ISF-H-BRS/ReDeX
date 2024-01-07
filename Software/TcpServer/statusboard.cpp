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

#include "assertions.h"
#include "exception.h"
#include "statusboard.h"

#include <array>

// ---------------------------------------------------------------------------------------------- //

using namespace std::chrono_literals;

// ---------------------------------------------------------------------------------------------- //

StatusBoard::StatusBoard(const QString& port)
    : m_serialPort("/dev/" + port, this),
      m_timer(this)
{
    if (!m_serialPort.open(QSerialPort::WriteOnly))
        throw Exception("Unable to open serial port " + port + ".");

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(sendStatus()));

    m_timer.setInterval(1s);
    m_timer.setSingleShot(true);
    m_timer.start();
}

// ---------------------------------------------------------------------------------------------- //

void StatusBoard::setStatus(Status status)
{
    m_status = status;
    sendStatus();
}

// ---------------------------------------------------------------------------------------------- //

void StatusBoard::sendStatus()
{
    static constexpr std::array<const char*, StatusCount> commands = {
        "<IDLE>\r\n", "<ACTIVE>\r\n", "<ALARM>\r\n"
    };

    const auto index = static_cast<size_t>(m_status);
    ASSERT(index < StatusCount);

    m_serialPort.write(commands[index]);
    m_timer.start();
}

// ---------------------------------------------------------------------------------------------- //
