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

#include "devicemanager.h"

#include <QObject>
#include <QTimer>

class PowerMonitor : public QObject
{
    Q_OBJECT

public:
    explicit PowerMonitor(const DeviceManager& devices);

public slots:
    void start();
    void stop();

    void setNotificationEnabled(bool enable);

signals:
    void alarmStatusChanged(size_t severityIndex);
    void recordAvailable(const QString& record);
    void error(const QString& msg);

private slots:
    void update();

private:
    void processHubStatus();
    void processNodeStatus(const Node& node);
    auto checkAlarmThresholds(const Node& node) -> size_t;

    void emitHubAlarm(Alarm::Type type, Alarm::Severity severity);

private:
    const DeviceManager& m_devices;
    bool m_notificationEnabled = false;
    QTimer m_timer;
    size_t m_currentSeverity = Alarm::indexOf(Alarm::Severity::None);
};
