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

#include "messagedispatcher.h"
#include "parsererror.h"
#include "protocol.h"

// ---------------------------------------------------------------------------------------------- //

MessageDispatcher::MessageDispatcher(QObject* parent)
    : QObject(parent) {}

// ---------------------------------------------------------------------------------------------- //

void MessageDispatcher::process(const QByteArray& data)
{
    for (char byte : data)
    {
        m_currentData.append(byte);

        if (m_currentData.endsWith(Protocol::LineBreak))
        {
            m_currentData.chop(Protocol::LineBreakLength);

            const QString data = QString::fromUtf8(m_currentData);
            m_currentData.clear();

            parseData(data);
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

void MessageDispatcher::parseData(const QString& data)
{
    QStringList tokens = data.split(Protocol::TokenSeparator);

    const QString tag = tokens.takeFirst();

    if (tag == "<GET_NODE_INFO>")
        emit nodeInfoRequested();
    else if (tag == "<GET_TESTPOINT_INFO>")
        emit testpointInfoRequested();
    else if (tag == "<START_POWER_MONITOR>")
        emit startPowerMonitorReceived();
    else if (tag == "<STOP_POWER_MONITOR>")
        emit stopPowerMonitorReceived();
    else if (tag == "<START_MEASUREMENT>")
        emit startMeasurementReceived();
    else if (tag == "<STOP_MEASUREMENT>")
        emit stopMeasurementReceived();
    else
        throw ParserError("Client sent an invalid message tag \"" + tag + "\".");

    if (!tokens.isEmpty())
        throw ParserError("Client sent supernumerous request tokens.");
}

// ---------------------------------------------------------------------------------------------- //
