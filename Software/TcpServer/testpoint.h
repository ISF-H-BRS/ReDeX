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

#include "conductancesensor.h"
#include "orpsensor.h"
#include "phsensor.h"
#include "potentiostatsensor.h"
#include "temperaturesensor.h"

#include <QObject>

#include <memory>

class Testpoint;
using TestpointPtr = std::unique_ptr<Testpoint>;

class Testpoint : public QObject
{
    Q_OBJECT

public:
    struct Sensors
    {
        ConductanceSensor* conductance = nullptr;
        OrpSensor* orp = nullptr;
        PhSensor* ph = nullptr;
        PotentiostatSensor* potentiostat = nullptr;
        TemperatureSensor* temperature = nullptr;
    };

public:
    Testpoint(const QString& id, const Sensors& sensors);

    auto id() const -> const QString&;

    auto conductanceSensor() const -> ConductanceSensor*;
    auto orpSensor() const -> OrpSensor*;
    auto phSensor() const -> PhSensor*;
    auto potentiostatSensor() const -> PotentiostatSensor*;
    auto temperatureSensor() const -> TemperatureSensor*;

signals:
    void conductanceAvailable(const QString& id,
                              double voltage, double current, double admittance);
    void orpValueAvailable(const QString& id, double value);
    void phValueAvailable(const QString& id, double value);
    void voltammogramAvailable(const QString& id, const Voltammogram& data);
    void temperatureAvailable(const QString& id, double value);

private slots:
    void processConductance(double voltage, double current, double admittance);
    void processOrpValue(double value);
    void processPhValue(double value);
    void processVoltammogram(const Voltammogram& data);
    void processTemperature(double value);

private:
    QString m_id;

    ConductanceSensor* m_conductanceSensor = nullptr;
    OrpSensor* m_orpSensor = nullptr;
    PhSensor* m_phSensor = nullptr;
    PotentiostatSensor* m_potentiostatSensor = nullptr;
    TemperatureSensor* m_temperatureSensor = nullptr;
};
