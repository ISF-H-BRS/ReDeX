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

#include "alarm.h"

#include <QObject>
#include <QString>

#include <chrono>
#include <memory>

using namespace std::chrono_literals;

class Node;
using NodePtr = std::unique_ptr<Node>;

class Node : public QObject
{
    Q_OBJECT

public:
    static constexpr std::chrono::milliseconds UpdateInterval = 500ms;

    struct Status
    {
        double voltage = 0.0;
        double current = 0.0;
        double temperature = 0.0;
    };

public:
    auto id() const -> const QString&;
    auto type() const -> const QString&;

    auto currentStatus() const -> const Status&;
    auto lastStatus() const -> const Status&;

    virtual auto alarmThresholds() const -> const Alarm::Thresholds& = 0;

    virtual void startMeasurement() = 0;
    virtual void stopMeasurement() = 0;

    virtual void update() = 0;

protected:
    Node(const QString& id, const QString& type);
    void updateStatus(const Status& status);

private:
    const QString m_id;
    const QString m_type;

    Status m_currentStatus = {};
    Status m_lastStatus = {};
};
