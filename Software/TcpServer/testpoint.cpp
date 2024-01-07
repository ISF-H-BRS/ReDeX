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
#include "testpoint.h"

// ---------------------------------------------------------------------------------------------- //

Testpoint::Testpoint(const QString& id, const Sensors& sensors)
    : m_id(id),
      m_conductanceSensor(sensors.conductance),
      m_orpSensor(sensors.orp),
      m_phSensor(sensors.ph),
      m_potentiostatSensor(sensors.potentiostat),
      m_temperatureSensor(sensors.temperature)
{
    ASSERT(!id.isEmpty());

    if (m_conductanceSensor)
    {
        connect(m_conductanceSensor, SIGNAL(valuesAvailable(double,double,double)),
                this, SLOT(processConductance(double,double,double)));
    }

    if (m_orpSensor)
        connect(m_orpSensor, SIGNAL(valueAvailable(double)), this, SLOT(processOrpValue(double)));

    if (m_phSensor)
        connect(m_phSensor, SIGNAL(valueAvailable(double)), this, SLOT(processPhValue(double)));

    if (m_potentiostatSensor)
    {
        connect(m_potentiostatSensor, SIGNAL(dataAvailable(Voltammogram)),
                this, SLOT(processVoltammogram(Voltammogram)));
    }

    if (m_temperatureSensor)
    {
        connect(m_temperatureSensor, SIGNAL(valueAvailable(double)),
                this, SLOT(processTemperature(double)));
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Testpoint::id() const -> const QString&
{
    return m_id;
}

// ---------------------------------------------------------------------------------------------- //

auto Testpoint::conductanceSensor() const -> ConductanceSensor*
{
    return m_conductanceSensor;
}

// ---------------------------------------------------------------------------------------------- //

auto Testpoint::orpSensor() const -> OrpSensor*
{
    return m_orpSensor;
}

// ---------------------------------------------------------------------------------------------- //

auto Testpoint::phSensor() const -> PhSensor*
{
    return m_phSensor;
}

// ---------------------------------------------------------------------------------------------- //

auto Testpoint::potentiostatSensor() const -> PotentiostatSensor*
{
    return m_potentiostatSensor;
}

// ---------------------------------------------------------------------------------------------- //

auto Testpoint::temperatureSensor() const -> TemperatureSensor*
{
    return m_temperatureSensor;
}

// ---------------------------------------------------------------------------------------------- //

void Testpoint::processConductance(double voltage, double current, double admittance)
{
    emit conductanceAvailable(m_id, voltage, current, admittance);
}

// ---------------------------------------------------------------------------------------------- //

void Testpoint::processOrpValue(double value)
{
    emit orpValueAvailable(m_id, value);
}

// ---------------------------------------------------------------------------------------------- //

void Testpoint::processPhValue(double value)
{
    emit phValueAvailable(m_id, value);
}

// ---------------------------------------------------------------------------------------------- //

void Testpoint::processVoltammogram(const Voltammogram& data)
{
    emit voltammogramAvailable(m_id, data);
}

// ---------------------------------------------------------------------------------------------- //

void Testpoint::processTemperature(double value)
{
    if (m_phSensor)
        m_phSensor->setTemperature(value);

    emit temperatureAvailable(m_id, value);
}

// ---------------------------------------------------------------------------------------------- //
