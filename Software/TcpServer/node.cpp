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
#include "node.h"

// ---------------------------------------------------------------------------------------------- //

Node::Node(const QString& id, const QString& type)
    : m_id(id),
      m_type(type)
{
    ASSERT(!id.isEmpty());
    ASSERT(!type.isEmpty());
}

// ---------------------------------------------------------------------------------------------- //

auto Node::id() const -> const QString&
{
    return m_id;
}

// ---------------------------------------------------------------------------------------------- //

auto Node::type() const -> const QString&
{
    return m_type;
}

// ---------------------------------------------------------------------------------------------- //

auto Node::currentStatus() const -> const Status&
{
    return m_currentStatus;
}

// ---------------------------------------------------------------------------------------------- //

auto Node::lastStatus() const -> const Status&
{
    return m_lastStatus;
}

// ---------------------------------------------------------------------------------------------- //

void Node::updateStatus(const Status& status)
{
    m_lastStatus = m_currentStatus;
    m_currentStatus = status;
}

// ---------------------------------------------------------------------------------------------- //
