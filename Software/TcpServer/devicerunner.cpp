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
#include "devicerunner.h"
#include "protocol.h"

// ---------------------------------------------------------------------------------------------- //

DeviceRunner::DeviceRunner(const DeviceManager& devices)
    : m_devices(devices),
      m_timer(this)
{
    m_timer.setInterval(Node::UpdateInterval);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));

    const std::vector<Testpoint*>& testpoints = devices.testpoints();

    for (auto t : testpoints)
    {
        connect(t, SIGNAL(conductanceAvailable(QString,double,double,double)),
                this,  SLOT(processConductance(QString,double,double,double)));

        connect(t, SIGNAL(orpValueAvailable(QString,double)),
                this,  SLOT(processOrpValue(QString,double)));

        connect(t, SIGNAL(phValueAvailable(QString,double)),
                this,  SLOT(processPhValue(QString,double)));

        connect(t, SIGNAL(voltammogramAvailable(QString,Voltammogram)),
                this,  SLOT(processVoltammogram(QString,Voltammogram)));

        connect(t, SIGNAL(temperatureAvailable(QString,double)),
                this,  SLOT(processTemperature(QString,double)));
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::start()
{
    m_timer.start();
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::stop()
{
    m_timer.stop();
    stopMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::startMeasurement()
{
    RETURN_IF_NOT(m_timer.isActive());

    const std::vector<Node*>& nodes = m_devices.nodes();

    try {
        for (auto node : nodes)
            node->startMeasurement();
    }
    catch (const std::exception& e) {
        emit error(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::stopMeasurement()
{
    const std::vector<Node*>& nodes = m_devices.nodes();

    try {
        for (auto node : nodes)
            node->stopMeasurement();
    }
    catch (const std::exception& e) {
        emit error(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::update()
{
    const std::vector<Node*>& nodes = m_devices.nodes();

    try {
        for (auto node : nodes)
            node->update();
    }
    catch (const std::exception& e) {
        emit error(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::processConductance(const QString& id,
                                      double voltage, double current, double admittance)
{
    const QString values = Protocol::joinValues("%2", "%3", "%4");
    const QString record = Protocol::joinTokens("<CONDUCTANCE>", "%1", values);

    emit recordAvailable(record.arg(id).arg(voltage).arg(current).arg(admittance));
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::processOrpValue(const QString& id, double value)
{
    const QString record = Protocol::joinTokens("<ORP>", "%1", "%2");
    emit recordAvailable(record.arg(id).arg(value));
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::processPhValue(const QString& id, double value)
{
    const QString record = Protocol::joinTokens("<PH>", "%1", "%2");
    emit recordAvailable(record.arg(id).arg(value));
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::processVoltammogram(const QString& id, const Voltammogram& data)
{
    static const auto addValue = [](double sample, QString& string) {
        string += Protocol::ValueSeparator + QString::number(sample);
    };

    RETURN_IF(data.current.size() != data.voltage.size());

    const size_t size = data.voltage.size();

    if (size == 0)
        return;

    auto voltage = QString::number(data.voltage[0]);
    auto current = QString::number(data.current[0]);

    for (size_t i = 1; i < size; ++i)
    {
        addValue(data.voltage[i], voltage);
        addValue(data.current[i], current);
    }

    const QString record = Protocol::joinTokens("<VOLTAMMOGRAM>", id, voltage, current);
    emit recordAvailable(record);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceRunner::processTemperature(const QString& id, double value)
{
    const QString record = Protocol::joinTokens("<TEMPERATURE>", "%1", "%2");
    emit recordAvailable(record.arg(id).arg(value));
}

// ---------------------------------------------------------------------------------------------- //
