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
#include <QString>
#include <QTimer>

class DeviceRunner : public QObject
{
    Q_OBJECT

public:
    explicit DeviceRunner(const DeviceManager& devices);

public slots:
    void start();
    void stop();

    void startMeasurement();
    void stopMeasurement();

signals:
    void recordAvailable(const QString& record);
    void error(const QString& msg);

private slots:
    void update();

    void processConductance(const QString& id,
                            double voltage, double current, double admittance);
    void processOrpValue(const QString& id, double value);
    void processPhValue(const QString& id, double value);
    void processVoltammogram(const QString& id, const Voltammogram& data);
    void processTemperature(const QString& id, double value);

private:
    const DeviceManager& m_devices;
    QTimer m_timer;
};
