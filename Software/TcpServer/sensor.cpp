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
#include "sensor.h"

// ---------------------------------------------------------------------------------------------- //

Sensor::Sensor(const QString& id, Node* node, size_t input)
    : m_id(id),
      m_node(node),
      m_input(input)
{
    ASSERT(!id.isEmpty());
    ASSERT_NOT_NULL(node);
}

// ---------------------------------------------------------------------------------------------- //

auto Sensor::id() const -> const QString&
{
    return m_id;
}

// ---------------------------------------------------------------------------------------------- //

auto Sensor::node() const -> Node*
{
    return m_node;
}

// ---------------------------------------------------------------------------------------------- //

auto Sensor::input() const -> size_t
{
    return m_input;
}

// ---------------------------------------------------------------------------------------------- //
