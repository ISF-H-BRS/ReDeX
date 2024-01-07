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

#include "node.h"

#include <QObject>
#include <QString>

#include <memory>

class Sensor;
using SensorPtr = std::unique_ptr<Sensor>;

class Sensor : public QObject
{
public:
    Sensor(const QString& id, Node* node, size_t input);
    ~Sensor() override = default;

    auto id() const -> const QString&;
    auto node() const -> Node*;
    auto input() const -> size_t;

protected:
    static auto outOfRange(auto value, auto min, auto max) -> bool {
        return value < min || value > max;
    };

private:
    QString m_id;
    Node* m_node;
    size_t m_input;
};
